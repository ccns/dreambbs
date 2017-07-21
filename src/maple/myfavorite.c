/*-------------------------------------------------------*/
/* maple/myfavorite.c	( ES_BBS )	 	         */
/*-------------------------------------------------------*/
/* author : cat.bbs@bbs.es.ncku.edu.tw                   */
/* target : «Ø¥ß [§Úªº³Ì·R°Ï] 				 */
/* create : 2005/07/28				 	 */
/* update : 					 	 */
/* note   : for ccns bbs				 */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef	HAVE_FAVORITE
extern BCACHE *bshm;

extern XZ xz[];
extern char brd_bits[MAXBOARD];
extern int ok;

static void XoFavorite(char *folder,char *title,int level);
static int myfavorite_add();
static char currdir[64];

time_t brd_visit[MAXBOARD];	/* ³ÌªñÂsÄý®É¶¡ */

void
brd2myfavorite(brd, gem)
  BRD *brd;
  HDR *gem;
{
  memset(gem, 0, sizeof(HDR));
  time(&gem->chrono);
  strcpy(gem->xname, brd->brdname);
  sprintf(gem->title, "%-16s%s", brd->brdname, brd->title );
  gem->xmode = GEM_BOARD;
}

static void
myfavorite_item(num, myfavorite)
  int num;
  HDR *myfavorite;
{
  if(myfavorite->xmode & GEM_BOARD)
  {
    if(myfavorite->recommend == -1)
      prints("%6d   %-13s< ¥»¬ÝªO¤w¤£¦s¦b >\n",num,myfavorite->xname);
    else
    {
      BRD *brd;
      char str[20],buf[10],*bits,*str2,buf2[20],brdtype;
      int chn,brdnew, bno;
      
      chn = myfavorite->recommend;
      brd = bshm->bcache + chn;
      
      brh_get(brd->bstamp, chn);
      
//      if(cuser.ufo2 & UFO2_ENHANCE)
//        sprintf(str,"%c\033[1;32m%c\033[m",(brd->readlevel & PERM_SYSOP) ? ')' : ' ',brd->blast > brd_visit[chn] ? '+' : ' ');
//      else
//        sprintf(str,"%c\033[1;32m%c\033[m",(brd->readlevel & PERM_SYSOP) ? ')' : ' ',brh_unread(brd->blast) ? '+' : ' ');
      brdnew = cuser.ufo2 & UFO2_BRDNEW;
      bits = brd_bits;
  
      sprintf(buf,"%d;3%d",brd->color/10,brd->color%10);

	if (!brdnew)
	{
	  int fd, fsize;
	  char folder[64];
	  struct stat st;

	  brd_fpath(folder, brd->brdname, fn_dir);
	  if ((fd = open(folder, O_RDONLY)) >= 0)
	  {
	    fstat(fd, &st);

          if(st.st_mtime > brd->btime)  // ¤W¦¸²Î­p«á¡AÀÉ®×¦³­×§ï¹L
          {
            if((fsize = st.st_size) >= sizeof(HDR))
            {
              HDR hdr;

              brd->bpost = fsize / sizeof(HDR);
              while ((fsize -= sizeof(HDR)) >= 0)
              {
                lseek(fd, fsize, SEEK_SET);
                read(fd, &hdr, sizeof(HDR));
                if (!(hdr.xmode & (POST_LOCK | POST_BOTTOM)))
                  break;
              }
              brd->blast = hdr.chrono;
            }
           else
              brd->blast = brd->bpost = 0;
          }

	    close(fd);
	  }
                                   
    }
      
    else
	{
	  int fd, fsize;
	  char folder[64];
	  struct stat st;

	  brd_fpath(folder, brd->brdname, fn_dir);
	  if ((fd = open(folder, O_RDONLY)) >= 0)
	  {
	    fstat(fd, &st);

          if(st.st_mtime > brd->btime)  // ¤W¦¸²Î­p«á¡AÀÉ®×¦³­×§ï¹L
          {
            if((fsize = st.st_size) >= sizeof(HDR))
            {
              HDR hdr;

              brd->bpost = fsize / sizeof(HDR);
              while ((fsize -= sizeof(HDR)) >= 0)
              {
                lseek(fd, fsize, SEEK_SET);
                read(fd, &hdr, sizeof(HDR));
                if (!(hdr.xmode & (POST_LOCK | POST_BOTTOM)))
                  break;
              }
              brd->blast = (hdr.chrono > brd->blast) ? hdr.chrono : brd->blast;
            }
           else
              brd->blast = brd->bpost = 0;
          }

	    close(fd);
	  }
	  num = brd->bpost;                                      
    }

    sprintf(str,"%s",brd->blast > brd_visit[chn] ? "\033[1;31m¡¹\033[m" : "¡¸");  

/* 081122.cache:¬ÝªO©Ê½è,¤£­q¾\,¯µ±K,¦n¤Í,¤@¯ë */
      if(bits[chn] & BRD_Z_BIT)
        brdtype = '-';
#ifdef HAVE_MODERATED_BOARD
      else if(brd->readlevel & PERM_BOARD)
        brdtype = '.';     
      else if(brd->readlevel & PERM_SYSOP)
        brdtype = ')';
#endif
      else
        brdtype = ' ';

/* ³B²z ¤H®ð */ /* cache.20090416: ¥épttÅÜ¦â*/
      bno = brd - bshm->bcache;
      bno = bshm->mantime[bno];
      str2 = buf2;

      if (bno > 4999)
        str2 = "\033[1;32m TOP \033[m";
      else if (bno > 999)
        str2 = "\033[1;32m  Éq\033[m ";
      else if (bno > 799)
        str2 = "\033[1;35m  Éq\033[m ";
      else if (bno > 699)
        str2 = "\033[1;33m  Éq\033[m ";
      else if (bno > 599)
        str2 = "  \033[1;42mÃz\033[m ";
      else if (bno > 499)
        str2 = "  \033[1;45mÃz\033[m ";
      else if (bno > 449)
        str2 = "  \033[1;44mÃz\033[m ";
      else if (bno > 399)
        str2 = "\033[1;32m  Ãz\033[m ";
      else if (bno > 349)
        str2 = "\033[1;35m  Ãz\033[m ";
      else if (bno > 299)
        str2 = "\033[1;33m  Ãz\033[m ";
      else if (bno > 249)
        str2 = "\033[1;36m  Ãz\033[m ";
      else if (bno > 199)
        str2 = "\033[1;31m  Ãz\033[m ";
      else if (bno > 149)
        str2 = "\033[1;37m  Ãz\033[m ";
      else if (bno > 99)
        str2 = "\033[1;31m HOT\033[m ";
      else if (bno > 49)
        str2 = "\033[1;37m HOT\033[m ";
      else if (bno > 5)
        sprintf(str2, "  %2d ", bno);
      else
        str2 = "     ";
//ª`·N¦³¤T®æªÅ¥Õ, ¦]¬° HOT ¬O¤T­Ó char ¬G§ó§ï±Æª©

      //sprintf(buf,"%d;3%d",brd->color/10,brd->color%10);
          prints("%6d%s%c%-13s\033[%sm%-4s \033[m%-33.32s%s%.13s\n", num, str,
          brdtype,brd->brdname, buf, brd->class, brd->title, brd->bvote ? "\033[1;33m  §ë\033[m " : str2, brd->BM);      
    
      //prints("%6d%s%c%-13s\033[%sm%-4s \033[m%-36.36s%c %.13s\n", num, str,
        //bits[chn] & BRD_Z_BIT ? '-' : ' ',brd->brdname,
        //buf,brd->class, brd->title, brd->bvote ? 'V' : ' ', brd->BM);
  
    }
  }
  else if(myfavorite->xmode & GEM_GOPHER)
  {
    prints("%6d   ¡½ %s ºëµØ°Ï±¶®|\n",num,myfavorite->xname);
  }
  else if(myfavorite->xmode & GEM_HTTP)
  {
    prints("  ----------------------------------------------------------------------------\n");
  }
  else
  {
    prints("%6d   %s\n",num,myfavorite->title);
  }
}

static int
myfavorite_body(xo)
  XO *xo;
{
  HDR *myfavorite;
  int num, max, tail;

  move(3, 0);
  clrtobot();
  max = xo->max;
  if (max <= 0)
  {
    return myfavorite_add(xo);
  }

  myfavorite = (HDR *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;
    
  do
  {
    myfavorite_item(++num, myfavorite++);
  } while (num < max);

  return XO_NONE;
}


static int
myfavorite_head(xo)
  XO *xo;
{
  vs_head("§Úªº³Ì·R", str_site);
  prints("[¡ö]¥D¿ï³æ [¡÷]¾\\Åª [¡ô¡õ]¿ï¾Ü [c]½g¼Æ [/]·j´M [s]¬ÝªO [h]»¡©ú\n"
    "\033[44m  %-7s¬Ý  ªO            %-33s¤H®ð ªO    ¥D     \033[m",
    cuser.ufo2 & UFO2_BRDNEW ? "Á`¼Æ" : "½s¸¹", "¤¤   ¤å   ±Ô   ­z");
  //outs("[¡ö]Â÷¶} [¡÷]¶i¤J [^P]·s¼W [d]§R°£ [c]¤Á´« [d]§R°£ [h]»¡©ú\n[37;44m  ");
  //outs(cuser.ufo2 & UFO2_BRDNEW ? "Á`¼Æ" : "½s¸¹");
  //outs("   ¬Ý  ªO       ¤¤   ¤å   ±Ô   ­z                     §ë²¼ ªO    ¥D     [m");
  return myfavorite_body(xo);
}

static int
myfavorite_newmode(xo)
  XO *xo;
{
  cuser.ufo2 ^= UFO2_BRDNEW;  /* Thor.980805: ¯S§Oª`·N utmp.ufoªº¦P¨B°ÝÃD */
  return myfavorite_head(xo);
}


static int
myfavorite_load(xo)
  XO *xo;
{
  myfavorite_main();
  xo_load(xo, sizeof(HDR));
  return myfavorite_body(xo);
}


static int
myfavorite_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return myfavorite_head(xo);
}

static int
myfavorite_switch(xo)
  XO *xo;
{
  Select();
  return myfavorite_load(xo);
}

static int
myfavorite_browse(xo)
  XO *xo;
{
  HDR *ghdr;
  int xmode,op=0,chn;
  char fpath[80], title[TTLEN + 1];
  
  ghdr = (HDR *) xo_pool + (xo->pos - xo->top);

  xmode = ghdr->xmode;
  /* browse folder */
  
  if( xmode & GEM_HTTP)
    return XO_NONE;

  if (xmode & GEM_BOARD)
  {
    if(ghdr->recommend == -1)
      return XO_NONE;
    chn = brd_bno(ghdr->xname);
    XoPost(chn);
    if(ok == 1)
    {
      xover(XZ_POST);
      time(&brd_visit[chn]); 
    }
  }
  else if(xmode & GEM_GOPHER)
  {
    sprintf(fpath,"gem/brd/%s/.DIR",ghdr->xname);

	chn = brd_bno(ghdr->xname);/*20100916.float ¶i¤JºëµØ°Ïµ¥¦P¶i¤J¬ÝªO*/
    XoPost(chn);				/*¨¾¤î§Q¥Î¨ä¥L¬ÝªOªºª©¥DÅv­­­×§ï¥Lª©ºëµØ°Ï ©ÎÆ[¬ÝÂê©w¤å³¹*/

    op = HAS_PERM(PERM_ALLBOARD) ? GEM_SYSOP : (bbstate & STAT_BOARD ? GEM_MANAGER : GEM_USER);
    strcpy(title, ghdr->title);
    XoGem(fpath, title, op);
    return myfavorite_init(xo);
  }
  else
  {
    char buf[20];
    
    op = xo->key;

    sprintf(buf,"MF/%s",ghdr->xname);
    usr_fpath(fpath,cuser.userid,buf);
    strcpy(title, ghdr->title);
    XoFavorite(fpath, title, op);
    return myfavorite_init(xo);
  }
  return myfavorite_init(xo);
}

static int
myfavorite_find_same(brd,dir)
  BRD *brd;
  char *dir;
{
  int max,i;
  HDR hdr;
  
  max = rec_num(dir,sizeof(HDR));

  for(i=0;i<max;i++)
  {
    rec_get(dir,&hdr,sizeof(HDR),i);
    if(!strcmp(hdr.xname,brd->brdname) && hdr.xmode & GEM_BOARD)
      return i;
  }
  return -1;
}

static int
myfavorite_add(xo)
  XO *xo;
{
  char ans;
  char buf[20];
  BRD *brd;
  HDR hdr;

  if(!HAS_PERM(PERM_VALID))
    return XO_NONE;

  memset(&hdr,0,sizeof(HDR));
  ans = vans("·s¼W (B)¬ÝªO±¶®| (F)¸ê®Æ§¨ (G)ºëµØ°Ï±¶®| (L)¤À¹j½u (Q)Â÷¶} [Q]");
  
  if(ans == 'b')
  {
    brd = ask_board(buf, BRD_R_BIT, NULL);
    if(brd == NULL)
    {
      vmsg("¿ù»~ªº¬ÝªO¦WºÙ");
      return XO_HEAD;
    }
    brd2myfavorite(brd,&hdr);
    if(myfavorite_find_same(brd,currdir) >= 0)
    {
      vmsg("¤w¦³¦¹¬ÝªO!");
      return XO_FOOT;
    }
  }
  else if(ans == 'f')
  {
    char title[64];
    char fpath[64];
    
    if(!vget(b_lines,0,"½Ð¿é¤J¼ÐÃD: ",title,sizeof(title),DOECHO))
      return XO_NONE;
      
    hdr_stamp(currdir, ans|HDR_LINK , &hdr, fpath);
    hdr.xmode = GEM_FOLDER;
    sprintf(hdr.title,"¡» %s",title);
  }
  else if(ans == 'g')
  {
    brd = ask_board(buf, BRD_R_BIT, NULL);
    if(brd == NULL)
    {
      vmsg("¿ù»~ªº¬ÝªO¦WºÙ");
      return XO_HEAD;
    }
    brd2myfavorite(brd,&hdr);
    hdr.xmode = GEM_GOPHER;
    
  }
  else if(ans == 'l')
  {
    hdr.xmode = GEM_HTTP;
    //sprintf(hdr.title,"¡» %s",title);
  }
  else
  {
    if(!xo->max)
      return XO_QUIT;
    return XO_FOOT;
  }
  
  ans = vans("¦s©ñ¦ì¸m A)ppend I)nsert N)ext Q)uit [A] ");

  if (ans == 'q')
  {
    return XO_FOOT;
  }

  if (ans == 'i' || ans == 'n')
    rec_ins(currdir, &hdr, sizeof(HDR), xo->pos + (ans == 'n'), 1);
  else
    rec_add(currdir, &hdr, sizeof(HDR));
    
  logitfile(FN_FAVORITE_LOG,"< ADD >",hdr.xname);

  return myfavorite_load(xo);
}

static void
remove_dir(fpath)
  char *fpath;
{
  HDR hdr;
  int max,i;
  char buf[20],path[80];

  max = rec_num(fpath,sizeof(HDR));
  for(i=0;i<max;i++)
  {
    rec_get(fpath,&hdr,sizeof(HDR),i);
    if(hdr.xmode & GEM_FOLDER)
    {
      sprintf(buf,"MF/%s",hdr.xname);
      usr_fpath(path,cuser.userid,buf);
      remove_dir(path);
    }
  }
  unlink(fpath);
}

static int
myfavorite_delete(xo)
  XO *xo;
{
  if(!HAS_PERM(PERM_VALID))
    return XO_NONE;

  if (vans(msg_del_ny) == 'y')
  {
    HDR *hdr;
    
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    
    if(hdr->xmode & GEM_FOLDER)
    {
      char buf[20];
      char fpath[64];
      
      sprintf(buf,"MF/%s",hdr->xname);
      usr_fpath(fpath,cuser.userid,buf);
      remove_dir(fpath);
    }
    
    if (!rec_del(currdir, sizeof(HDR), xo->pos, NULL, NULL))
    {
      logitfile(FN_FAVORITE_LOG,"< DEL >",hdr->xname);
      return myfavorite_load(xo);
    }
  }
  return XO_FOOT;
}

static int
myfavorite_mov(xo)
  XO *xo;
{
  HDR *ghdr;
  char buf[80];
  int pos, newOrder;

  if(!HAS_PERM(PERM_VALID))
    return XO_NONE;
  ghdr = (HDR *) xo_pool + (xo->pos - xo->top);

  pos = xo->pos;
  sprintf(buf + 5, "½Ð¿é¤J²Ä %d ¿ï¶µªº·s¦ì¸m¡G", pos + 1);
  if (!vget(b_lines, 0, buf + 5, buf, 5, DOECHO))
    return XO_FOOT;

  newOrder = atoi(buf) - 1;
  if (newOrder < 0)
    newOrder = 0;
  else if (newOrder >= xo->max)
    newOrder = xo->max - 1;

  if (newOrder != pos)
  {
    if (!rec_del(currdir, sizeof(HDR), pos, NULL, NULL))
    {
      rec_ins(currdir, ghdr, sizeof(HDR), newOrder, 1);
      xo->pos = newOrder;
      logitfile(FN_FAVORITE_LOG,"< MOV >",ghdr->xname);
      return XO_LOAD;
    }
  }
  return XO_FOOT;
}

static int
myfavorite_edit(xo)
  XO *xo;
{
  HDR *hdr;
  
  if(!HAS_PERM(PERM_VALID))
    return XO_NONE;
  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  if(hdr->xmode & GEM_BOARD)
  {
    int chn;

    if(!HAS_PERM(PERM_BOARD))
      return XO_NONE;

    chn = hdr->recommend;
    if (chn >= 0)
    {
      brd_edit(chn);
      return myfavorite_init(xo);
    }
    
  }
  else if(hdr->xmode & GEM_FOLDER)
  {
    if(!vget(b_lines,0,"½Ð¿é¤J¼ÐÃD: ",hdr->title,64,GCARRY))
      return XO_FOOT;
    rec_put(currdir,hdr,sizeof(HDR),xo->pos);
    return myfavorite_load(xo);
  }
  return XO_NONE;
    
}


static int
myfavorite_help(xo)
  XO *xo;
{
//  film_out(FILM_FAVORITE, -1);
  return myfavorite_head(xo);
}

static int
myfavorite_search(xo)
  XO *xo;
{
  int num, pos, max;
  char *ptr;
  char buf[IDLEN + 1];
  HDR *hdr;

  ptr = buf;
  pos = vget(b_lines, 0, "½Ð¿é¤J·j´MÃöÁä¦r¡G", ptr, IDLEN + 1, DOECHO);
  move(b_lines, 0);
  clrtoeol();

  if (pos)
  {
    int chn;
    BRD *brd;

    //bcache = bshm->bcache;

    //str_lower(ptr, ptr);
    pos = num = xo->pos;
    max = xo->max;
    //chp = (short *) xo->xyz;
    do
    {
      if (++pos >= max)
	pos = 0;

      hdr = (HDR *) xo_pool + (pos - xo->top);
	
      if(hdr->xmode & GEM_BOARD)
      {
        chn = hdr->recommend;
        brd = bshm->bcache + chn;
        //vmsg(ptr);
        
        if(strstr(brd->brdname, ptr) || strstr(brd->title, ptr))
          return pos + XO_MOVE;
      }
      else if(hdr->xmode & GEM_FOLDER)
      {
        if(strstr(hdr->title, ptr))
          return pos + XO_MOVE;
      }
      //chn = chp[pos];
      //if (chn >= 0)
      //{
	//brd = bcache + chn;
	//if (str_str(brd->brdname, ptr) || str_str(brd->title, ptr))
	  //return pos + XO_MOVE;
      //}
    } while (pos != num);
  }

  return XO_NONE;
}


KeyFunc myfavorite_cb[] =
{
#ifdef	HAVE_LIGHTBAR
  XO_ITEM, myfavorite_item_bar,
#endif
  XO_INIT, myfavorite_init,
  XO_LOAD, myfavorite_load,
  XO_HEAD, myfavorite_head,
  XO_BODY, myfavorite_body,

  Ctrl('P'), myfavorite_add,
  'r', myfavorite_browse,
  's', myfavorite_switch,
  'c', myfavorite_newmode,
  'd', myfavorite_delete,
  'M', myfavorite_mov,
  'E', myfavorite_edit,
  '/', myfavorite_search,
  'h', myfavorite_help
};

static void
XoFavorite(folder, title, level)
  char *folder;
  char *title;
  int level;
{
  XO *xo, *last;
  char old[64];

  last = xz[XZ_MYFAVORITE - XO_ZONE].xo;     /* record */

  strcpy(old,currdir);
  
  strcpy(currdir,folder);

  xz[XZ_MYFAVORITE - XO_ZONE].xo = xo = xo_new(folder);
  xz[XZ_MYFAVORITE - XO_ZONE].cb = myfavorite_cb;
  xo->pos = 0;
  xo->key = XZ_MYFAVORITE;

  xover(XZ_MYFAVORITE);

  free(xo);
  
  strcpy(currdir,old);

  xz[XZ_MYFAVORITE - XO_ZONE].xo = last;     /* restore */
}


int
MyFavorite()
{
  char fpath[64];

  utmp_mode(M_MYFAVORITE);
  usr_fpath(fpath,cuser.userid,FN_MYFAVORITE);
  myfavorite_main();

  XoFavorite(fpath, "§Úªº³Ì·R", XZ_MYFAVORITE);

  return XO_HEAD;
}

int 
myfavorite_find_chn(brdname)
  char *brdname;
{
  BRD *bp;
  int max,i;
//  char *userid,bm[40];
  
  bp = bshm->bcache;
  max = bshm->number;
  
  for(i=0;i<max;i++,bp++)
  {
    if(!strcmp(bp->brdname,brdname))
    {
    /*
      strcpy(bm,bp->BM);
      if(strstr(bm,cuser.userid))
      {
        userid = (char *) strtok(bm,"/");
        do
        {
          if(!strcmp(cuser.userid,userid))
            return i;
        } while(userid = (char *) strtok(NULL,"/"));
      }
      else 
      */
      if(HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_ALLBOARD) /* || !(bp->readlevel & PERM_SYSOP) || bm_belong(brdname) & BRD_R_BIT || (bp->battr & BRD_FRIEND) */ || (Ben_Perm(bp, cuser.userlevel) & (BRD_R_BIT|BRD_F_BIT|BRD_X_BIT)))
        return i;
      else
        break;
    }
  }
  
  return -1;
}

void
myfavorite_parse(fpath)
  char *fpath;
{
  int i,max;
  char buf[20];
  HDR hdr;

  sprintf(buf,"MF/%s",fpath);
  
  usr_fpath(fpath,cuser.userid,buf);
  max = rec_num(fpath,sizeof(HDR));
  
  for(i=0;i<max;i++)
  {
    rec_get(fpath,&hdr,sizeof(HDR),i);
    if(hdr.xmode & GEM_BOARD)
    {
      hdr.recommend = myfavorite_find_chn(hdr.xname);
      rec_put(fpath,&hdr,sizeof(HDR),i);
    }
    else if(hdr.xmode & GEM_FOLDER)
    {
      myfavorite_parse(hdr.xname);
    }
  }

}


void
myfavorite_main()
{
  int i,max;
  char fpath[80];
  HDR hdr;
  
  usr_fpath(fpath, cuser.userid, "MF");
  if(!mkdir(fpath, 0700))
  {
    char old[80],new[80],cmd[128];
    usr_fpath(old,cuser.userid,FN_FAVORITE);
    usr_fpath(new,cuser.userid,FN_MYFAVORITE);
    sprintf(cmd,"cp %s %s", old, new);
    system(cmd);
  }

  usr_fpath(fpath,cuser.userid,FN_MYFAVORITE);
  max = rec_num(fpath,sizeof(HDR));
  
  for(i=0;i<max;i++)
  {
    rec_get(fpath,&hdr,sizeof(HDR),i);
    if(hdr.xmode & GEM_BOARD)
    {
      hdr.recommend = myfavorite_find_chn(hdr.xname);
      rec_put(fpath,&hdr,sizeof(HDR),i);
    }
    else if(hdr.xmode & GEM_FOLDER)
    {
      myfavorite_parse(hdr.xname);
    }
  }
}

int
class_add(xo)
  XO *xo;
{
  HDR hdr;
  char fpath[64];
  short chn,*chp;
  BRD *brd;
  
  usr_fpath(fpath,cuser.userid,FN_MYFAVORITE);
  
  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  if(chn < 0)
  {
    return XO_NONE;
  }

  brd = bshm->bcache + chn;

  if(myfavorite_find_same(brd,fpath) >= 0)
  {
    vmsg("¤w¦³¦¹¬ÝªO!");
    return XO_FOOT;
  }

  memset(&hdr,0,sizeof(HDR));
  brd2myfavorite(brd,&hdr);
  
  rec_add(fpath,&hdr,sizeof(HDR));
  logitfile(FN_FAVORITE_LOG,"< ADD >",hdr.xname);
  vmsg("¤w¥[¤J§Úªº³Ì·R");
                          
  return XO_FOOT;
}

#endif	/* HAVE_FAVORITE */

