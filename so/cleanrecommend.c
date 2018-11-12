/*-------------------------------------------------------*/
/* cleanrecommend.c   ( CCNS BBS )  		         */
/*-------------------------------------------------------*/
/* author : cat@ccns.ncku.edu.tw                         */
/* target : cleanrecommend                               */
/* create : 2004/12/12                                   */
/* update : NULL                                         */
/*-------------------------------------------------------*/

#undef	_MODES_C_
#include "bbs.h"

#define NEGATIVE 0
#define POSITIVE 1
#define COMMENT  2

#ifdef	HAVE_RECOMMEND
typedef struct RecommendMessage
{
  char userid[IDLEN + 1];
  char verb[3];
  int  pn;
  char msg[55];
  char rtime[6];
}       RMSG;


extern BCACHE *bshm;
extern XZ xz[];
int counter;
char title[80],name[10];

static int
cleanrecommend_log(rmsg,mode)
  RMSG *rmsg;
  int mode;	/* 0:partial 1:all */
{
  FILE *fp;
  time_t now;
  
  if(fp = fopen(FN_RECOMMEND_LOG,"a+"))
  {
    time(&now);
    
    fprintf(fp,"%24.24s %s 砍 %s 板 %s(%s) ",ctime(&now),cuser.userid,currboard,title,name);
    if(!mode)
      fprintf(fp,"中 %s 的留言 %s\n",rmsg->userid,rmsg->msg);
    else
      fprintf(fp,"所有留言\n");
    fclose(fp);
  }

  return 0;
}

static void
cleanrecommend_item(num, cleanrecommend)
  int num;
  RMSG *cleanrecommend;
{
    
    char tmp[10],*pn;
    
    pn = tmp;
    
    if(cleanrecommend->pn == POSITIVE)
    {
      pn = "\033[1;33m+";
      prints("%4d%s%2s\033[m%-12s %-54s%-5s\n", num,pn,cleanrecommend->verb,cleanrecommend->userid,cleanrecommend->msg,cleanrecommend->rtime);
    }      
    else if(cleanrecommend->pn == NEGATIVE)
    {
      pn = "\033[1;31m-";
      prints("%4d%s%2s\033[m%-12s %-54s%-5s\n", num,pn,cleanrecommend->verb,cleanrecommend->userid,cleanrecommend->msg,cleanrecommend->rtime);      
    }
    else
    {
      pn = "   ";
      prints("%4d%s%2s\033[m%-12s %-54s%-5s\n", num,pn,cleanrecommend->verb,cleanrecommend->userid,cleanrecommend->msg,cleanrecommend->rtime);      
    }
}

static int
cleanrecommend_body(xo)
  XO *xo;
{
  RMSG *cleanrecommend;
  int num, max, tail;

  move(3, 0);
  clrtobot();
  max = xo->max;
  if (max <= 0)
  {
    //counter = 0;
    vmsg("沒有留言");
    return XO_QUIT;
  }
  cleanrecommend = (RMSG *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
/*
  if((counter = max) > 127)
    counter = 127;
    
  if(counter < -127)
    counter = -127;
*/    
  if (max > tail)
    max = tail;

  do
  {
    cleanrecommend_item(++num, cleanrecommend++);
  } while (num < max);
      
  return XO_NONE;
}


static int
cleanrecommend_head(xo)
  XO *xo;
{
  vs_head("推薦留言清單", str_site);
  outs("\
  [←]離開 c)修改[站長專用] d)刪除 D)清除全部 s)重整 [h]elp\n\
\033[44m編號 推      使用者 留言                                                  日 期\x1b[m");
  return cleanrecommend_body(xo);
}


static int
cleanrecommend_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(RMSG));
  return cleanrecommend_body(xo);
}


static int
cleanrecommend_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(RMSG));
  return cleanrecommend_head(xo);
}


static int
cleanrecommend_edit(cleanrecommend,echo)
  RMSG *cleanrecommend;
  int echo;
{
  if(echo == DOECHO)
    memset(cleanrecommend, 0, sizeof(RMSG));
  if(vget(b_lines, 0, "使用者:", cleanrecommend->userid, sizeof(cleanrecommend->userid), echo)
   && vget(b_lines, 0, "動詞:",cleanrecommend->verb, sizeof(cleanrecommend->verb), echo)
   && vget(b_lines, 0, "留言:",cleanrecommend->msg, sizeof(cleanrecommend->msg), echo)
   && vget(b_lines, 0, "日期:",cleanrecommend->rtime, sizeof(cleanrecommend->rtime), echo))
    return 1;
  else
    return 0;
}


static int
cleanrecommend_delete(xo)
  XO *xo;
{

  if (vans(msg_del_ny) == 'y')
  {
    RMSG *rmsg;
    int pos, cur;
    
    pos = xo->pos;
    cur = pos - xo->top;
    rmsg = (RMSG *) xo_pool + cur;
    
    if (!rec_del(xo->dir, sizeof(RMSG), xo->pos, NULL, NULL))
    {
      cleanrecommend_log(rmsg,0);
      return cleanrecommend_load(xo);
    }
  }
  return XO_FOOT;
}

static int
cleanrecommend_change(xo)
  XO *xo;
{
  RMSG *cleanrecommend, mate;
  int pos, cur;
  
  if(!HAS_PERM(PERM_BOARD))
    return XO_NONE;
  
  pos = xo->pos;
  cur = pos - xo->top;
  cleanrecommend = (RMSG *) xo_pool + cur;

  mate = *cleanrecommend;
  cleanrecommend_edit(cleanrecommend, GCARRY);
  if (memcmp(cleanrecommend, &mate, sizeof(RMSG)))
  {
    rec_put(xo->dir, cleanrecommend, sizeof(RMSG), pos);
    move(3 + cur, 0);
    cleanrecommend_item(++pos, cleanrecommend);
  }

  return XO_FOOT;
}

static int
cleanrecommend_cleanall(xo)
  XO *xo;
{
  if(vans("確定要刪除所有的留言嗎？[y/N]") == 'y')
  {
    unlink(xo->dir);
    cleanrecommend_log(NULL,1);
    return cleanrecommend_load(xo);
  }
  return XO_FOOT;
}

static int
cleanrecommend_help(xo)
  XO *xo;
{
//  film_out(FILM_RMSG, -1);
  return cleanrecommend_head(xo);
}

KeyFunc cleanrecommend_cb[] =
{
  XO_INIT, cleanrecommend_init,
  XO_LOAD, cleanrecommend_load,
  XO_HEAD, cleanrecommend_head,
  XO_BODY, cleanrecommend_body,

  'c', cleanrecommend_change,
  's', cleanrecommend_init,
  'd', cleanrecommend_delete,
  'D', cleanrecommend_cleanall,
  'h', cleanrecommend_help
};

int
clean(xo)
  XO *xo;
{
  XO *xoo;
  HDR *hdr,phdr;
  int pos, cur;
  char fpath[128],buf[256],tmp[128],recommenddb[128],*c2;
  FILE *fp;  
  RMSG rmsg;
  int i,chrono,pushstart;
  struct stat st;
  int total, fd;
  BRD *brd;
  unsigned int battr;

  counter = 0;
  pushstart = 0;

  if (!(bbstate & STAT_BOARD))
    return 0;

    pos = xo->pos;
    cur = pos - xo->top;
    hdr = (HDR *) xo_pool + cur;

    if(!hdr->recommend || hdr->xmode & (POST_DELETE | POST_CANCEL | POST_MDELETE | POST_LOCK | POST_CURMODIFY))
      return XO_NONE;
      
    chrono = hdr->chrono;
    strcpy(title,hdr->title);
    strcpy(name,hdr->xname);
      
    hdr_fpath(fpath,xo->dir,hdr);
    sprintf(recommenddb,"tmp/%s.recommenddb",cuser.userid);
    sprintf(tmp,"tmp/%s.clean",cuser.userid);
    unlink(tmp);
    unlink(recommenddb);

    brd = bshm->bcache + brd_bno(currboard);
    battr = brd->battr;

    if(fp = fopen(fpath,"r"))
    {
            
/*
      if(brd->battr & BRD_PUSHSNEER)
      {
        if (addscore == 1)
          sprintf(add,                "[[1;33m→ %12s：[[36m%-54.54s [[m%5.5s\n",cuser.userid,msg,Btime(&hdr->pushtime)+3);
        else if (addscore == -1)
          sprintf(add,      "[[1;31m噓[[m [[1;33m%12s：[[36m%-54.54s [[m%5.5s\n",cuser.userid,msg,Btime(&hdr->pushtime)+3);
      }
      else if(brd->battr & BRD_PUSHDEFINE)
      {
        if(addscore == 1)
          sprintf(            add,"[[1;33m%02.2s %12s：[[36m%-54.54s [[m%5.5s\n",verb,cuser.userid,msg,Btime(&hdr->pushtime)+3);
        else if (addscore == -1)
          sprintf(  add,"[[1;31m%02.2s[[m [[1;33m%12s：[[36m%-54.54s [[m%5.5s\n",verb,cuser.userid,msg,Btime(&hdr->pushtime)+3);
        else
          sprintf(add,                "[[1;33m→ %12s：[[36m%-54.54s [[m%5.5s\n",cuser.userid,msg,Btime(&hdr->pushtime)+3);
      }
      else
        sprintf(add,                  "[[1;33m→ %12s：[[36m%-54.54s [[m%5.5s\n",cuser.userid,msg,Btime(&hdr->pushtime)+3);
*/
        while(fgets(buf,256,fp))
        {
          memset(&rmsg, 0, sizeof(RMSG));
          if(!strncmp(buf,"\033[1;32m※",9))
            pushstart = 1;
          
          if(pushstart)
          {
            if(!strncmp(buf,"\033[1;32m※",9))
            {
              f_cat(tmp,buf);
              continue;
            }
            c2 = strrchr(buf,'\n') - 5;
            strncpy(rmsg.rtime, c2, 5);
            
            c2 -= 58;
            strncpy(rmsg.msg, c2, 54);
            
            c2 -= 19;
            strncpy(rmsg.userid, c2, 12);
            
            c2 = strchr(buf, 'm');
            strncpy(rmsg.verb, c2+1, 2);
            
            if((battr & BRD_PUSHDEFINE) && !strncmp(rmsg.verb,"→",2) )
              rmsg.pn = COMMENT;
            else if(!strncmp(rmsg.verb,"\033[m\033[1;33",2))
		      rmsg.pn = COMMENT;			
            /*else if(strncmp(buf, "\033[1;33→", 8))
		      rmsg.pn = POSITIVE;*/
			else
              rmsg.pn = !strncmp(buf, "\033[1;33", 6);

            rec_add(recommenddb,&rmsg,sizeof(RMSG));
//          if(!strncmp(buf,"\x1b[1;33m→",9))
//          {
/*
            for(i=0;i<12;i++)
              rmsg.userid[i] = buf[i+10];
            rmsg.userid[12] = '\0';
            for(i=0;i<54;i++)
              rmsg.msg[i] = buf[i+29];
            rmsg.msg[54] = '\0';
            for(i=0;i<5;i++)
              rmsg.rtime[i] = buf[i+87];
            rmsg.rtime[5] = '\0';
            rec_add(recommenddb,&rmsg,sizeof(RMSG));
*/
//          }
            
          }
          else
            f_cat(tmp,buf);
        }
      fclose(fp);
    }

  xz[XZ_OTHER - XO_ZONE].xo = xoo = xo_new(recommenddb);
  xz[XZ_OTHER - XO_ZONE].cb = cleanrecommend_cb;
  xover(XZ_OTHER);
  free(xoo);
  
  for(i=0;i<rec_num(recommenddb,sizeof(RMSG));i++)
  {
    rec_get(recommenddb,&rmsg,sizeof(RMSG),i);
    if(rmsg.pn == POSITIVE)
    {
      counter++;
      sprintf(buf,"\x1b[1;33m%2s %12s：\x1b[36m%-54.54s \x1b[m%5.5s\n",rmsg.verb,rmsg.userid,rmsg.msg,rmsg.rtime);
    }
    else if(rmsg.pn == NEGATIVE)
    {
      counter--;
      sprintf(buf,"\x1b[1;31m%2s \033[33m%12s：\x1b[36m%-54.54s \x1b[m%5.5s\n",rmsg.verb,rmsg.userid,rmsg.msg,rmsg.rtime);
    }
    else
    {
      sprintf(buf,"\033[m\033[1;33m   %12s：\033[36m%-54.54s \033[m%5.5s\n",rmsg.userid,rmsg.msg,rmsg.rtime);
    }
    f_cat(tmp,buf);
  }
  
  if(dashf(tmp))
  {
    sprintf(buf,"mv %s %s",tmp,fpath);
    system(buf);
  }

  if((fd = open(xo->dir, O_RDWR, 0600)) == -1)
    return XO_NONE;
  
  fstat(fd, &st);
  total = st.st_size / sizeof(HDR);
  if(pos > total)
    pos = total;  
  
  f_exlock(fd);
  while(pos >= -1)
  {
    lseek(fd, (off_t) (sizeof(HDR) * pos--), SEEK_SET);
    read(fd,&phdr,sizeof(HDR));
    if(chrono == phdr.chrono)
      break;
  }

  if(++pos >= 0)  
  {
    phdr.recommend = counter;
    phdr.xmode &= ~POST_RECOMMEND;
    phdr.xid = 0;
    lseek(fd, (off_t) (sizeof(HDR) * pos), SEEK_SET);
    write(fd,&phdr,sizeof(HDR));
//    rec_put(xo->dir, &phdr, sizeof(HDR), pos);
  }

  f_unlock(fd);
  close(fd);

  return XO_INIT;
}
#endif
