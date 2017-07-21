/*-------------------------------------------------------*/
/* popupmenu.c   ( YZU_CSE WindTop BBS )                 */
/*-------------------------------------------------------*/
/* author : verit.bbs@bbs.yzu.edu.tw                     */
/* target : popup menu                     		 */
/* create : 2003/02/12                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

static screenline sl[24];

static int do_menu(MENU pmenu[],XO *xo,int x,int y);

/* ----------------------------------------- */
/* verit �P�_�r�ꪺ�� pos �U�r���O���@�ر��p */
/*     return 1  �N����r���e�b��          */
/*           -1  �N����r����b��          */
/*            0  �N���O����r              */
/* ----------------------------------------- */

static int 
is_big5(char *src,int pos,int mode)
{
    int wstate=0;
    int word=0;
    char *str;

    for(str = src;word<pos;str++)
    {
       if(mode)
       {
         if(*str == '\033')
         {
           str = strchr(str,'m');
           if(!str)
             return -1;
           continue ;
         }
       }

       wstate = (*str<0)?((wstate==1)?0:1):0;
       word++;
    }
    
    if(wstate)
      return -1;

    if(*str<0 && wstate==0)
      return 1;
    else
      return 0;
}

static int 
do_cmd(MENU *mptr,XO *xo,int x,int y)
{
   usint mode;
   void *p;
   int (*func) ();

   if(mptr->umode < 0)
   {
      p = DL_get(mptr->func);
      if(!p) 
        return 0;
      mptr->func = p;
      mptr->umode = - (mptr->umode);
   }

   switch (mptr->umode)
   {
      case POPUP_SO :
        p = DL_get(mptr->func);
        if(!p) return 0;
        mptr->func = p;
        mptr->umode = POPUP_FUN;
        func = p;
        mode = (*func) ();
        return -1;
      case POPUP_MENU :
//        sprintf(t,"�i%s�j",mptr->desc);
        vs_restore(sl);
        return do_menu((MENU *)mptr->func,xo,x,y);
      case POPUP_XO :
        func = mptr->func;
        mode = (*func) (xo);
        return -1;
      case POPUP_FUN :
        func = mptr->func;
        mode = (*func) ();
        return -1;
   }
   
   return -1;
}

/* verit . �p�⦩����X����ڪ��� */
static int 
count_len(data)
  char *data;
{
  int len;
  char *ptr,*tmp;
  ptr = data;
  len = strlen(data);

  while(ptr)
  {
    ptr = strstr(ptr,"\033");
    if(ptr)
    {
      for(tmp=ptr;*tmp!='m';tmp++);
      len -= (tmp-ptr+1);
      ptr = tmp+1;
    }
  }
  return len;
}

/* verit . ���o�C�� */
static void
get_color(char *s,int len,int *fc,int *bc,int *bbc)
{
   char buf[32],*p,*e;
   int color;
   int state = 0,reset=0,exit = 0;
   
   memset(buf,0,sizeof(buf));
   strncpy(buf,s+2,len-1);
   
   for( p = e = &buf[0] ; exit == 0 ; ++p)
   {
       if(*p == ';' || *p == 'm')
       {
         if(*p == 'm')
            exit = 1;
         *p = 0;
         
         color = atoi(e);
         
         if(color == 0)
         {
            *bbc = 0;
            reset = 1;
         }
         else if(color > 0 && color < 10)
         {
            *bbc = color;
            state |= 0x1;
         }
         else if(color>29 && color < 38)
         {
            *fc = color;
            state |= 0x2;
         }
         else if(color>39 && color < 48)
         {
            *bc = color;
            state |= 0x4;
         }
         e = p+1;
       }
   }
   
   if(reset == 1)
   {
     if(!(state & 0x4))
       *bc = 40;
     if(!(state & 0x2))
       *fc = 37;
     if(!(state & 0x1))
       *bbc = 0;
   }
   
   if(state == 0)
   {
     *bc = 40;
     *fc = 37;
     *bbc = 0;
   }
   
}

static void
vs_line(char *msg,int x,int y)
{
   char buf[512],color[16],*str,*tmp,*cstr;
   int len = count_len(msg);
   int word,slen,fc=37,bc=40,bbc=0;
   
   memset(buf,0,sizeof(buf));
   
   sl[x].data[sl[x].len] = '\0';
   str = tmp = sl[x].data;
   
   for(word=0;word<y && *str;++str)
   {
      if(*str == KEY_ESC)
      {
         cstr = strchr(str,'m');
         get_color(str,cstr-str,&fc,&bc,&bbc);
         str = cstr;
         continue;
      }
      word++;
   }
   
   strncpy(buf,sl[x].data, str - tmp);
   
   while(word++<y)
     strcat(buf," ");
   
   slen = strlen(buf)-1;
   
   /* verit . ���p�o������r�����e�b�X�N�M�� */
   if(is_big5(buf,slen,0)>0)
     buf[slen] = ' ';
     
   strcat(buf,msg);
   
   if(*str)
   {
      for(word=0;word<len && *str;++str)
      {
        if(*str == KEY_ESC)
        {
           cstr = strchr(str,'m');
           get_color(str,cstr-str,&fc,&bc,&bbc);
           str = cstr;
           continue;
        }
        word++;
      }
      
      if(*str)
      {
        /* verit . �ѨM�᭱�C��ɦ� */
        sprintf(color,"\033[%d;%d;%dm",bbc,fc,bc);
        strcat(buf,color);

        /* verit . ���p�̫�@�r�������媺��b���N�M�� */
        slen = strlen(buf);
        strcat(buf,str);
        if(is_big5(tmp,str-tmp,0)<0)
          buf[slen] = ' ';
      }
   }

   move(x,0);
   outs(buf);
   
}

/* mode �N��[�I�����C�� */
static void
draw_item(char *desc,int mode,int x,int y)
{
   char buf[128];

   sprintf(buf," \033[0;37m�j\033[4%d;37m%s(\033[1;36m%c\033[0;37;4%dm)%-25s%s\033[0;47;30m�p\033[40;30;1m�p\033[m ",mode,(mode>0)?"�t":"  ",*desc,mode,desc+1,(mode>0)?"�u":"  ");
   vs_line(buf,x,y);
   move(b_lines,0);
}


static int 
draw_menu(MENU *pmenu[20],int num,char *title,int x,int y,int cur)
{
   char buf[128];
   int i;
   char t[128];
   
   sprintf(t,"�i%s�j",title);
   
   sprintf(buf," \033[0;40;37m�ššššššššššššššššš�\033[m   ");
   vs_line(buf,x-2,y);
   sprintf(buf," \033[0;37;44m�j\033[1m%-31s \033[0;47;34m�p\033[m   ",t);
   vs_line(buf,x-1,y);
   for(i=0;i<num;++i,++x)
   {
      draw_item(pmenu[i]->desc,(i==cur)?1:0,x,y);
   }
   sprintf(buf," \033[0;47;30m�h\033[30;1m�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h\033[40;30;1m�p\033[m ");
   vs_line(buf,x,y);
   return 0;
}

static int 
do_menu(pmenu,xo,x,y)
  MENU pmenu[];
  XO *xo;
  int x;
  int y;
{
   int cur,old_cur,num,tmp;
   char c;
   MENU *table[20];
   char *title;
   MENU *table_title;

   memset(table ,0 ,sizeof(table));   
   /* verit. menu �v���ˬd */
   for( tmp=0,num=-1 ; pmenu[tmp].umode != POPUP_MENUTITLE ; tmp++ )
   {
     if(pmenu[tmp].level == 0 || pmenu[tmp].level & cuser.userlevel)
       table[++num] = &pmenu[tmp];
       
   }
   
   /* verit. ��s�ʺA */
   if(pmenu[tmp].func)
   {
     strcpy(cutmp->mateid,pmenu[tmp].func);
     utmp_mode(M_IDLE);
   }
   
   title = pmenu[tmp].desc;
   table_title = &pmenu[tmp];

   /* verit . ���p�S������ﶵ�N return */   
   if(num < 0)
     return 1;
     
   cur = old_cur = 0;  
     
   /* ����w�]�ﶵ */
   for( tmp=0; tmp<= num ; tmp++ )
   {
      if((table[tmp]->desc[0] | 0x20) == ((table_title->level & POPUP_MASK) | 0x20))
      {
        cur = old_cur = tmp;
        break;
      }
   }
     
     
   draw_menu(table,num+1,title,x,y,cur);

   while(1)  /* verit . user ��� */
   {
      c = vkey();
      old_cur = cur;
      switch(c)
      {
         case KEY_LEFT:
            return 1;
            break;
         case KEY_UP:
            cur = (cur==0)?num:cur-1;
            break;
         case KEY_DOWN:
            cur = (cur==num)?0:cur+1;
            break;
         case KEY_HOME:
            cur = 0;
            break;
         case KEY_END:
            cur = num;
            break;
         case KEY_RIGHT:
         case '\n':
            if(table[cur]->umode & POPUP_QUIT)
                return 1;
            if(do_cmd(table[cur],xo,x,y)<0)
                return -1;
            vs_restore(sl);
            draw_menu(table,num+1,title,x,y,cur);
            break;
        default:
            for(tmp=0 ; tmp<=num ; tmp++)
            {
                if( (c | 0x20) == (table[tmp]->desc[0] | 0x20 ))
                {
                   cur = tmp;
                   if(table_title->level & POPUP_DO_INSTANT)
                   {
                     if(table[cur]->umode == POPUP_QUIT)
                        return 1;
                     if(do_cmd(table[cur],xo,x,y)<0)
                        return -1;
                     vs_restore(sl);
                     draw_menu(table,num+1,title,x,y,cur);
                   }
                   break;
                }
            }
            break;
      }
      if(old_cur != cur)
      {
        draw_item(table[old_cur]->desc,0,x+old_cur,y);
        draw_item(table[cur]->desc,1,x+cur,y);
      }
   }
   
   return 0;
}


/* mode �N��[�I�����C�� */
static void
draw_ans_item(desc,mode,x,y,hotkey)
  char *desc;
  int mode;
  int x;
  int y;
  char hotkey;
{
   char buf[128];

   sprintf(buf," \033[0;37m�j\033[4%d;37m%s%c\033[1;36m%c\033[0;37;4%dm%c%-25s%s\033[0;47;30m�p\033[40;30;1m�p\033[m ",mode,(mode>0)?"�t":"  ",(hotkey==*desc)?'[':'(',*desc,mode,(hotkey==*desc)?']':')',desc+1,(mode>0)?"�u":"  ");
   vs_line(buf,x,y);
   move(b_lines,0);
}


static int 
draw_menu_des(char *desc[],char *title,int x,int y,int cur)
{
   int num;
   char buf[128];
   char hotkey;
   hotkey = desc[0][0];
   
   sprintf(buf," \033[0;40;37m�ššššššššššššššššš�\033[m   ");
   vs_line(buf,x-2,y);
   sprintf(buf," \033[0;37;44m�j%-31s \033[0;47;34m�p\033[m   ",title);
   vs_line(buf,x-1,y);
   for(num=1;desc[num];num++)
      draw_ans_item(desc[num],(num==cur)?1:0,x++,y,hotkey);
   sprintf(buf," \033[0;47;30m�h\033[30;1m�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h\033[40;30;1m�p\033[m ");
   vs_line(buf,x,y);
   return num-2;
}

/*------------------------------------------------------------- */
/* verit . desc �Ĥ@�U��������� char , 			*/
/*   		�Ĥ@�U�r���N��@�}�l��а�����m		*/
/*              �ĤG�U�r���N����U KEY_LEFT ���w�]�^�ǭ�	*/
/*         desc �̫�@�U������ NULL 				*/
/*------------------------------------------------------------- */
int
popupmenu_ans(char *desc[],char *title,int x,int y)
{
   int cur,old_cur,num,tmp;
   char c;
   char t[64];
   char hotkey;
   hotkey = desc[0][0];
      
   vs_save(sl);
   sprintf(t,"�i%s�j",title);
   num = draw_menu_des(desc,t,x,y,0);
   cur = old_cur = 0;
   for(tmp=0;tmp<num;tmp++)
   {
     if(desc[tmp+1][0] == hotkey)
       cur = old_cur = tmp;
   }
   draw_ans_item(desc[cur+1],1,x+cur,y,hotkey);

   while(1)
   {
      c = vkey();
      old_cur = cur;
      switch(c)
      {
         case KEY_LEFT:
            vs_restore(sl);
            return (desc[0][1] | 0x20);
         case KEY_UP:
            cur = (cur==0)?num:cur-1;
            break;
         case KEY_DOWN:
            cur = (cur==num)?0:cur+1;
            break;
         case KEY_HOME:
            cur = 0;
            break;
         case KEY_END:
            cur = num;
            break;
         case KEY_RIGHT:
         case '\n':
            vs_restore(sl);
            return (desc[cur+1][0] | 0x20) ;
        default:
            for(tmp=0 ; tmp<=num ; tmp++)
            {
                if( (c | 0x20) == (desc[tmp+1][0] | 0x20 ))
                {
                   cur = tmp;
                   break;
                }
            }
            break;
      }
      if(old_cur != cur)
      {
        draw_ans_item(desc[old_cur+1],0,x+old_cur,y,hotkey);
        draw_ans_item(desc[cur+1],1,x+cur,y,hotkey);
      }
   }
   return 0;
}

void
popupmenu(MENU pmenu[],XO *xo,int x,int y)
{
   
   vs_save(sl);
   do_menu(pmenu,xo,x,y);
   vs_restore(sl);
}

static void pcopy(char *buf,char *patten,int len)
{
  int i,size;
  
  *buf = '\0';
  size = strlen(patten);
  
  for(i=1;i<=len;i++)
  {
    strcpy(buf,patten);
    buf += size;
  }
}

int pmsg(char *msg)
{
   char buf[ANSILINELEN];
   char patten[ANSILINELEN];
   int len,plen,cc;
   
   if(cuser.ufo2 & UFO2_ORIGUI)
     return vmsg(msg);
   

   vs_save(sl);
   
   len = (msg ? strlen(msg) : 0 );
   if(len > 30)
   {
     plen = (len - 29) / 2;
   }
   else
     plen = 0;
   
   if(len > 0)
   {
     pcopy(patten,"��",plen);
     sprintf(buf," \033[0;40;37m�šššššššššššššššš�%s\033[m  ",plen?patten:"");
     vs_line(buf,7,18-plen);
     pcopy(patten,"  ",plen);
     sprintf(buf," \033[0;37;44m�j �Ы����N���~�� .....         %s\033[0;47;34m�p\033[m ",plen?patten:"");
     vs_line(buf,8,18-plen);
     sprintf(buf," \033[0;37m�j                              %s\033[0;47;30m�p\033[m\033[40;30;1m�p\033[m ",plen?patten:"");
     vs_line(buf,9,18-plen);
     sprintf(buf," \033[0;37m�j%-30s%s\033[0;47;30m�p\033[m\033[40;30;1m�p\033[m ",msg,((len > 30 && len%2 == 1) ? " " : ""));
     vs_line(buf,10,18-plen);
     sprintf(buf," \033[0;37m�j                              %s\033[0;47;30m�p\033[m\033[40;30;1m�p\033[m ",plen?patten:"");
     vs_line(buf,11,18-plen);
     pcopy(patten,"�h",plen);
     sprintf(buf," \033[0;47;30m�h\033[30;1m�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h%s\033[40;30;1m�p\033[m ",plen?patten:"");
     vs_line(buf,12,18-plen);
   }
   else
   {
     sprintf(buf," \033[0;40;37m�ššššššššššššššššš�\033[m  ");
     vs_line(buf,7,18);
     sprintf(buf," \033[0;37;44m�j �Ы����N���~�� .....           \033[0;47;34m�p\033[m ");
     vs_line(buf,8,18);
     sprintf(buf," \033[0;37m�j                                \033[0;47;30m�p\033[m\033[40;30;1m�p\033[m ");
     vs_line(buf,9,18);
     sprintf(buf," \033[0;37m�j%-30s  \033[0;47;30m�p\033[m\033[40;30;1m�p\033[m ","[�Ы����N���~��]");
     vs_line(buf,10,18);
     sprintf(buf," \033[0;37m�j                                \033[0;47;30m�p\033[m\033[40;30;1m�p\033[m ");
     vs_line(buf,11,18);
     sprintf(buf," \033[0;47;30m�h\033[30;1m�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h�h\033[40;30;1m�p\033[m ");
     vs_line(buf,12,18);
   }

   cc = vkey();
   vs_restore(sl);
   return cc;
}


/* verit. 030211 ����ù��e����Ȧs�� */
int
Every_Z_Screen()
{
  FILE *fp;
  int i;
  char buf[512];

  fp = tbf_open();
  if(!fp)
  {
    vmsg("�ɮ׶}�ҿ��~ !!");
    return 0;
  }
  for(i=0;i<24;++i)
  {
     memset(buf,0,sizeof(buf));
     strncpy(buf,sl[i].data,sl[i].len);
     fprintf(fp,"%s\n",buf);
  }
  fclose(fp);
  return 1;
}

