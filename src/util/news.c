/*-------------------------------------------------------*/
/* news.c    (YZU WindTopBBS Ver 3.02 )                  */
/*-------------------------------------------------------*/
/* author : Verit.bbs@bbs.yzu.edu.tw			 */
/* modify : visor.bbs@bbs.yzu.edu.tw			 */
/* target : Kimo News                                    */
/* create : 01/07/12                                     */
/* update : 01/07/13					 */
/*-------------------------------------------------------*/

#include "bbs.h"
#include "news.h"

static char *spec_symbolic[]=
{
  "<lt;",
  ">gt;",
  "&amp;",
  "\"quot;",
  " nbsp;",
  NULL
};

int g_year,g_month,g_day;
char g_class[128];
char g_site[128];
char g_link[128];
char g_href[128];
char g_index[128];


NEWS *root;
PARSER *proot;

void 
syntax_parse(des,src)
  char *des;
  char *src;
{
  char *ptr,*str;
  if(!strncmp(src,"NULL",4))
  {
    des[0] = '\0';
    return;
  }
      
  for(ptr=des,str=src;*str;str++,ptr++)
  {
    if(*str == '\\')
    {
      switch(*++str)
      {
        case 'n':
          *ptr = '\n';
          break;
        case 't':
          *ptr = '\t';
          break;
        case '\\':
          *ptr = '\\';
          break;
        case 'r':
          *ptr = '\r';
          break;
        default:
          *ptr = *str;
      }
    }
    else
      *ptr = *str;
  }
  *ptr = '\0';
}

/* /{$DY}/{$DM}/{$DD}/{$CLASS}/{$SITE}/{$LINK} */
void 
var_parse(des,src)
  char *des;
  char *src;
{
  char buf[128],tag[16];
  char *ptr,*str;
  int i;
  
  for(ptr=des,str=src;*str;str++,ptr++)
  {
    if(*str == '{' && *(str+1) == '$')
    {
      memset(tag,0,16);
      str+=2;
      i=0;
      while(*str != '}')
      {
        tag[i++] = *str;
        str++;
      }
      
      if(!strcmp(tag,"DY"))
        sprintf(buf,"%04d",g_year);
      else if(!strcmp(tag,"DM"))
        sprintf(buf,"%02d",g_month);
      else if(!strcmp(tag,"DD"))
        sprintf(buf,"%02d",g_day);
      else if(!strcmp(tag,"CLASS"))
        sprintf(buf,"%s",g_class);
      else if(!strcmp(tag,"SITE"))
        sprintf(buf,"%s",g_site);
      else if(!strcmp(tag,"LINK"))
        sprintf(buf,"%s",g_link);
      else if(!strcmp(tag,"HREF"))
        sprintf(buf,"%s",g_href);
      else if(!strcmp(tag,"INDEX"))
        sprintf(buf,"%s",g_index);
      else
        buf[0] = '\0';
       
      strcpy(ptr,buf);
      ptr += strlen(buf)-1;
    }
    else
      *ptr = *str;
  }
  *ptr = '\0';  
  
}

PARSER *
get_parser(name)
  char *name;
{
  PARSER *cur;
  if(root->parser)
  {
    for(cur=root->parser;cur;cur=cur->next)
    {
      if(!strcmp(cur->name,name))
        return cur;
    }
  }
  return root->def;
}

void
ini_free(NEWS *ptr)
{
  PARSER *cur, *tmp = NULL;
  if(ptr->def)
    free(ptr->def);
  if(ptr->parser)
  {
    for(cur=ptr->parser;cur;cur=tmp)
    {
      tmp = cur->next;
      free(cur);
    }    
  }
  free(ptr);
}

NEWS *
load_ini(fpath)
  char *fpath;
{
  NEWS *ptr;
  PARSER *tmp = NULL, *cur;
  int global=0;
  int class_def=0;
  int class=0;
  char buf[256],*str;
  char name[128],value[512],cname[128];
  FILE *fp;
  fp = fopen(fpath,"r");
  if(fp)
  {
    ptr = (NEWS *)malloc(sizeof(NEWS));
    memset(ptr,0,sizeof(NEWS));
    while(fgets(buf,256,fp))
    {
      if(buf[0] == '#')
        continue;
      else if(!strncmp(buf,INI_GLOBAL,strlen(INI_GLOBAL)))
      {
        global = 1;
        class = class_def = 0;
        continue;
      }
      else if(!strncmp(buf,INI_DEFAULT,strlen(INI_DEFAULT)))
      {
        tmp = (PARSER *)malloc(sizeof(PARSER));
        memset(tmp,0,sizeof(PARSER));
        strcpy(tmp->name,"default");
        ptr->def = tmp;
        class_def = 1;
        global = class = 0;
        continue;
      }
      else if(!strncmp(buf,INI_CLASS,strlen(INI_CLASS)))
      {
        str = strtok(buf+strlen(INI_CLASS),"]");
        strcpy(cname,str);
        tmp = (PARSER *)malloc(sizeof(PARSER));
        memset(tmp,0,sizeof(PARSER));
        strcpy(tmp->name,cname);
        if(!ptr->parser)
          ptr->parser = tmp;
        else
        {
          for(cur = ptr->parser;cur->next;cur=cur->next);
          cur->next = tmp;
        }
        class = 1;
        class_def = global = 0;
        continue;
      }
      else if(!isalpha(buf[0]))
        continue;
      
      str = strtok(buf,"\t\n");
      
      if(str)
      {
        strcpy(name,str);
        str = strtok(NULL,"\t\n");
        if(str)
          strcpy(value,str);
      }
      
      if(global)
      {
        if(!strcmp(name,"SITE"))
          strcpy(ptr->site,value);
        else if(!strcmp(name,"NAME"))
          strcpy(ptr->name,value);
        else if(!strcmp(name,"HREF"))
          strcpy(ptr->href,value);
        else if(!strcmp(name,"SERVER"))
          strcpy(ptr->server,value);
        else if(!strcmp(name,"PROXY"))
          strcpy(ptr->proxy,value);
        else if(!strcmp(name,"PROXYPORT"))
          ptr->proxyport = atoi(value);
        else if(!strcmp(name,"PORT"))
          ptr->port = atoi(value);
        else if(!strcmp(name,"CLASS"))
        {
          str = strtok(value,",");
          strcpy(ptr->class[ptr->class_len++],str);
          while((str = strtok(NULL,",")))
          {
            strcpy(ptr->class[ptr->class_len++],str);
          }
        }
        else if(!strcmp(name,"INDEX"))
          strcpy(ptr->index,value);
        else if(!strcmp(name,"APATH"))
          strcpy(ptr->apath,value);
        else if(!strcmp(name,"PATH"))
          strcpy(ptr->path,value);
        else if(!strcmp(name,"DOC"))
          strcpy(ptr->doc,value);
        else
          continue;  
      }
      else if(class_def || class)
      {
        if(!strcmp(name,"LIST_START"))
          syntax_parse(tmp->l_start,value);
        else if(!strcmp(name,"LIST_END"))
          syntax_parse(tmp->l_end,value);
        else if(!strcmp(name,"LIST_HREFS"))
          syntax_parse(tmp->l_hrefs,value);
        else if(!strcmp(name,"LIST_HREFE"))
          syntax_parse(tmp->l_hrefe,value);
        else if(!strcmp(name,"LIST_TS"))
          syntax_parse(tmp->l_ts,value);
        else if(!strcmp(name,"LIST_TE"))
          syntax_parse(tmp->l_te,value);
        else if(!strcmp(name,"LIST_SHOW"))
          syntax_parse(tmp->l_show,value);
        else if(!strcmp(name,"LIST_UNSHOW"))
          syntax_parse(tmp->l_unshow,value);
        else if(!strcmp(name,"LIST_NEW"))
          syntax_parse(tmp->l_new,value);
        else if(!strcmp(name,"LIST_ART"))
          syntax_parse(tmp->l_art,value);
        else if(!strcmp(name,"ART_START"))
          syntax_parse(tmp->a_s,value);
        else if(!strcmp(name,"ART_END"))
          syntax_parse(tmp->a_e,value);
        else if(!strcmp(name,"ART_SHOW"))
          syntax_parse(tmp->a_show,value);
        else if(!strcmp(name,"ART_UNSHOW"))
          syntax_parse(tmp->a_unshow,value);
      }
      else
        continue;
    }
  
    fclose(fp);
    return ptr;
  }
  return NULL;
}


void
strncpytitle(des,src,len)
  char *des;
  char *src;
  int len;
{
  int word=0,wstate=0;
  char *ptr,*str;
  for(ptr = des,str = src; *str;str++)
  {
    if(word >= len-1 && *str<0 && wstate==0)
      break;

    word++;
    wstate = (*str<0)?((wstate==1)?0:1):0;
    *ptr++=*str;
  }
  *ptr='\0';
}

void
keeplogs(fnlog, board, title, site, index)
  char *fnlog;
  char *board;
  char *title;
  char *site;
  char *index;
{
  HDR hdr;
  char folder[256], fpath[256];
  int fd;
  FILE *fp;

  if (!board)
    board = BRD_SYSTEM;
  
  sprintf(folder, "%s/%s/.DIR", NEWS_ROOT,site);
  fd = hdr_stamp(folder, 'A', &hdr, fpath);
  if (fd < 0)
    return;

  fp = fdopen(fd, "w");
  fprintf(fp, "作者: %s (%s)\n標題: %-72.72s\n時間: %s\n",root->site,root->name,
    title, ctime(&hdr.chrono));
  f_suck(fp, fnlog);
  fprintf(fp, "--\n\033[1;32m※ Origin: \033[33m%s \033[37m<%s> \033[m\n\033[1;32m※ From  : \033[36m%s\033[m\n",
    root->name, root->server, index);
  fclose(fp);
  close(fd);
  unlink(fnlog);

  strncpytitle(hdr.title, title, 54);
  strcpy(hdr.owner, root->site);
  strcpy(hdr.nick, root->name);
  hdr.xid = hash32(title);

  sprintf(folder, "%s/%s/@/@%s",NEWS_ROOT,site, board);  
  if(rec_ins(folder,&hdr,sizeof(HDR),0,1))
    unlink(fpath);
}

void 
cut_title(title)
  char *title;
{
  char buf[512];
  char *ptr,*des;
  int i;
  for(ptr = title, des = buf;*ptr;ptr++)
  {
    if(*ptr != '\r' && *ptr != '\n' && *ptr != '\t')
    {
      if(*ptr == '&')
      {
        for(i=0;spec_symbolic[i] != NULL;i++)
        {
          if(!str_ncmp(ptr+1,&spec_symbolic[i][1],strlen(&spec_symbolic[i][1])))
          {
            *des++ = spec_symbolic[i][0];
            ptr += strlen(&spec_symbolic[i][1]);
            break;
          }
        }
      }
      else if(*ptr == '<')
      {
        for(;*ptr && *ptr != '>';ptr++);
      }
      else
        *des++ = *ptr;
    }
  }
  *des = '\0';
  strcpy(title,buf);
}


static int
http_conn2(char *server,int port ,char *s,char *fname,char *title,char *site, char *index)
{
  int sockfd;

  /* statue: state 解決中文字問題 ^^ */

  int cc, tlen,word=0,wstate=0,estate=0;
  int loop,total,size,state,i;
  char *xhead, *xtail, *pool, tag[128], *ptr, *ss = NULL, *se = NULL;
  FILE *fp;
  
  fd_set rd;
  struct timeval to;

  if ((sockfd = dns_open(server, port)) < 0)
  {
    return 0;
  }
  
  FD_ZERO(&rd);
  FD_SET(sockfd, &rd);
  to.tv_sec = 30;
  to.tv_usec = 0;
  pool = NULL;
  total = 0;

  write(sockfd, s, strlen(s));
  shutdown(sockfd, 1);

  /* parser return message from web server */
  while(1)
  {
      if(pool)
      {
        pool = (char *)realloc(pool,total+PAGE);
        xhead = pool + total;
      }
      else
      {
        xhead = pool = (char *)malloc(PAGE);
      }
      if(select(sockfd+1,&rd,NULL,NULL,&to)<=0)
      {
        printf("DEBUG: GET ARTICLE %s TIME OUT !!\n",index);
        free(pool);
        close(sockfd);
        return 0;
      }
      size = read(sockfd, xhead, PAGE);
      if (size <= 0)
        break;
      total+=size;
  }
  close(sockfd);
  pool[total] = '\0';
  
  //printf("DEBUG : article: %s\n",pool);
  xtail = pool + total;
  xhead = pool;
  state = 0;
  loop = 1;
    
  
  fp = fopen(NEWS_TMP_FILE, "w");
  fputc('\t',fp);

  while(loop)
  {

    switch(state)
    {
      case 0:
        if((ptr = strstr(xhead,proot->a_s)))
        {
          ss = xhead = ptr + strlen(proot->a_s);
          state = 1;
        }
        else
        {
          printf("DEBUG : state: %d\n",state);
          state = 4;
        }
        break;
      case 1:
        if(proot->a_unshow[0] && (ptr = strstr(xhead,proot->a_unshow)))
        {
          state = 2;
          se = ptr;
          xhead = ptr + strlen(proot->a_unshow);
        }
        else if((ptr = strstr(xhead,proot->a_e)))
        {
          state = 2;
          se = ptr;
          xhead = ptr + strlen(proot->a_e);
        }
        else
        {
          printf("DEBUG : state: %d\n",state);
          state = 4;        
        }
        break;
      case 2:
        tlen = 0;
        word = 0;
        for(ptr=ss;ptr<se;ptr++)
        {
          cc = *ptr;
          if (cc == '<' )
          {
            tlen = 1;
            continue;
          }

          if (tlen)
          {
            if (cc == '>')
            {
              if((tlen == 2) && (!str_ncmp(tag,"p",1)))
                fputc('\n',fp);
              tlen = 0;
              continue;
            }
            if (tlen <= 128)
            {
	      tag[tlen - 1] = cc;
            }
            tlen++;
            continue;
          }
          
          if(word > LINE_WORD && ((cc<0 && wstate==0) || (wstate==0 && estate == 0)))
          {
            fputc('\n',fp);
            fputc('\t',fp);
            word = 0 ;
          }

          if(cc == '&')
          {
            for(i=0;spec_symbolic[i] != NULL;i++)
            {
              if(!str_ncmp(ptr+1,&spec_symbolic[i][1],strlen(&spec_symbolic[i][1])))
              {
                cc = (char)spec_symbolic[i][0];
                ptr += strlen(&spec_symbolic[i][1]);
                break;
              }
            }
          }          
         
          if (cc != '\r' && cc != '\n' && cc != '\t')
          {
            word++;
            wstate = (cc<0)?((wstate==1)?0:1):0;
            estate = isgraph(cc);
  	    fputc(cc, fp);
          }
          else if(cc == '\n')
          {
            fputc('\n',fp);
            fputc('\t',fp);
            word = 0 ;
          }
 
        }
        state = 3;
        break;
      case 3:
        if(proot->a_show[0] && (ptr = strstr(xhead,proot->a_show)))
        {
          ss = xhead = ptr + strlen(proot->a_show);
          state = 1;
        }
        else
          state = 5;      
        break;
      case 4:
        printf("DEBUG: can't get : %s\n",index);
//	printf("DEBUG: msg : %s\n",pool);        
        free(pool);
        fclose(fp);
        return 0;
      case 5:
        loop = 0;
        break;
    }
    
  }
  fputc('\n', fp);
  fclose(fp);
  free(pool);
  keeplogs(NEWS_TMP_FILE,fname,title,site,index);
  return 0;
}

static int
news_found(fpath,value)
  char *fpath;
  int value;
{
  int fd;
  HDR hdr;

  fd = open(fpath,O_RDONLY);
  while(fd)
  {
    if (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if(hdr.xid == value)
      {
        close(fd);
        return 1;
      }
    }
    else
    {
      close(fd);
      break;
    }
  }
  return 0;
}

static int
http_conn(char *server, int port, char *s,char *fname,char *site,char *proxy,int proxyport)
{
  int sockfd;
  int hash_value;
  char tmp1[256],tmp2[256],ue_word[512],sendform[1024];
  char *pool, *xhead, *xtail,*ptr, *ss = NULL, *se;
  char title[256],index[256],href[256];
  int title_len,index_len,size,total,state,loop;
  char cserver[256],para[1024];

  fd_set rd;
  struct timeval to;  



  if(proxy)
  {
    if ((sockfd = dns_open(proxy, proxyport)) < 0)
    {
      return 0;
    }
  }
  else
  {
    if ((sockfd = dns_open(server, port)) < 0)
    {
      return 0;
    }
  }


  write(sockfd, s, strlen(s));
  shutdown(sockfd, 1);

  FD_ZERO(&rd);
  FD_SET(sockfd, &rd);
  to.tv_sec = 10;
  to.tv_usec = 0;

  /* parser return message from web server */
//  tlen = 0;
//  start_show = 0;
//  show = 0;
  title_len = 0;
  index_len = 0;
  pool = NULL;
  total = 0;
  loop = 1;
  size = 0;
  
  
  while(1)
  {
      if(pool)
      {
        pool = (char *)realloc(pool,total+PAGE);
        xhead = pool + total;
      }
      else
      {
        xhead = pool = (char *)malloc(PAGE);
      }
      if(select(sockfd+1,&rd,NULL,NULL,&to)<=0)
      {
        printf("DEBUG: GET CLASS %s TIME OUT !!\n",fname);
        free(pool);
        close(sockfd);
        return 0;
      }
      size = read(sockfd, xhead, PAGE);
      if (size <= 0)
	break;
      total+=size;
  }
  close(sockfd);

  pool[total] = '\0';
//  printf("DEBUG: total: %s\n",pool);
  xtail = pool + total;
  xhead = pool;
  state = 0;

#if 1
  while (loop)
  {
 
    switch(state)
    {
      case 0:
        if((ptr = strstr(xhead,proot->l_start)))
        {
          xhead = ptr + strlen(proot->l_start);
          if((ptr = strstr(xhead,proot->l_end)))
          {
            xtail = ptr;
          }
          state = 1;
        }
        else
          state = 7;
        break;
      case 1:
        if((ptr = strstr(xhead,proot->l_hrefs)) && ptr < xtail)
        {
          state = 2;
          ss = xhead =  ptr + strlen(proot->l_hrefs);
        }
        else if((ptr = strstr(xhead,proot->l_end)))
        {
          state = 7;
        }
        else
          state = 7;
        break;
      case 2:
        if((ptr = strstr(xhead,proot->l_hrefe)))
        {
          state = 3;
          se = ptr;
          xhead = ptr + strlen(proot->l_hrefe);
          strncpy(index,ss,se-ss);
          index[(index_len = se-ss)] = '\0';
          printf("DEBUG: index: %s\n",index);
        }
        else
          state = 7;
        break;
      case 3:
        if((ptr = strstr(xhead,proot->l_ts)))
        {
          state = 4;
          ss = xhead =  ptr + strlen(proot->l_ts);
        }
        else if((ptr = strstr(xhead,proot->l_end)))
        {
          state = 7;
        }
        else
          state = 7;        
        break;
      case 4:
        if((ptr = strstr(xhead,proot->l_te)))
        {
          state = 5;
          se = ptr;
          xhead = ptr + strlen(proot->l_te);
          strncpy(title,ss,se-ss);
          title[(title_len = se-ss)] = '\0';
          cut_title(title);
          printf("DEBUG: title: %s\n",title);
        } 
        else
          state = 7;
        break;
      case 5:
        if(title_len && index_len)
        {
          hash_value = hash32(title);
          strcpy(tmp2,g_class);
          tmp2[IDLEN-1] = '\0';

          sprintf(tmp1,"%s/%s/@/@%s",NEWS_ROOT,root->site,tmp2);

          if(!news_found(tmp1,hash_value))
          {
            printf("DEBUG: I NEED IT! : %s\n",title);
            if(!str_ncmp(index,"http://",strlen("http://")))
            {
              ptr = strtok(index+strlen("http://"),"/");
              strcpy(cserver,ptr);
              ptr = strtok(NULL,"\n");
              sprintf(tmp1,"/%s",ptr);
              strcpy(index,tmp1);
            }
            else if(index[0] == '.' && index[1] == '/')
            {
              strcpy(cserver,root->server);
              strcpy(tmp1,&index[1]);
              strcpy(index,tmp1);
            }
            else if(index[0] == '.' && index[1] == '.' && index[2] == '/')
            {
              strcpy(cserver,root->server);
              var_parse(tmp1,root->path);
              ptr = strrchr(tmp1,'/');
              *ptr = 0;
              ptr = strrchr(tmp1,'/');
              *(ptr+1) = 0;
              strcat(tmp1,&index[2]);
              strcpy(index,tmp1);
            }
            else
            {
              strcpy(cserver,root->server);
              if(index[0] != '/')
              {
                sprintf(tmp1,"/%s",index);
                strcpy(index,tmp1);
              }
            }
            url_encode(ue_word,index);
//            printf("DEBUG : link:%s\n",index);

			sprintf(para,PARA,cserver,port);

			if(proxy)
		        sprintf(sendform, "GET http://%s%s HTTP/1.1\r\n%s\r\n",cserver,ue_word,para);
			else
		        sprintf(sendform, "GET %s HTTP/1.1\r\n%s\r\n",ue_word,para);

			strcpy(g_link,index);
			
			
            var_parse(href,proot->l_art);
			if(proxy)
              http_conn2(proxy,proxyport,sendform,tmp2,title,root->site,href);
			else
 			  http_conn2(cserver,port,sendform,tmp2,title,root->site,href);

          }
          else
            printf("DEBUG: I HAVE IT! : %s\n",title);

          index_len = title_len = 0;
        }      
        state = 1;
        break;
      case 6:
        break;
      case 7:
        loop = 0;
        break;
    }

  }
#endif  
  free(pool);

  return 0;
}

int
main()
{
  char buf[128],ue_word[256],sendform[1024];
  char para[512];
  char folder[128],fpath[128];
  FILE *fp;
  struct tm *ptime;
  time_t now;
  int i;
  char *ptr,fsite[128],fname[128];
  
  
  chdir(BBSHOME);

  time(&now);
  ptime = localtime(&now);
  sprintf(fpath,"%s/%s",NEWS_ETC_ROOT,NEWS_SETUP_FILE);
  fp = fopen(fpath,"r");
  if(!fp)
  {
    printf("DEBUG: not found file:%s\n",fpath);
    exit(0);
  }
  
  while(fgets(buf,128,fp))
  {
    if(buf[0] == '#' || buf[0] == ' ' || buf[0] == '\n')
      continue;
    ptr = strtok(buf,"\t\r\n");
    if(ptr)
    {
      strcpy(fsite,ptr);
      ptr = strtok(NULL,"\t\r\n");
      if(ptr)
        strcpy(fname,ptr);
      else
        continue;
    }
    sprintf(fpath,"%s/%s",NEWS_ETC_ROOT,fname);
    if(root)
      ini_free(root);
    root = load_ini(fpath);
  
    for(i=0;i<root->class_len;i++)
    {
       proot = get_parser(root->class[i]);
       g_year = ptime->tm_year+1900;
       g_month = ptime->tm_mon + 1;
       g_day = ptime->tm_mday;
       strcpy(g_class,root->class[i]);
       strcpy(g_site,root->site);
       strcpy(g_href,root->href);
       strcpy(g_index,root->index);
       printf("DEBUG: site: %s  class: %s\n",root->site,root->class[i]);  
    
       memset(buf,0,sizeof(buf));  
       memset(ue_word,0,sizeof(ue_word));
       memset(sendform,0,sizeof(sendform));
       
       var_parse(buf,root->path);
       url_encode(ue_word,buf);
       memset(buf,0,sizeof(buf));
       sprintf(para,PARA,root->server,root->port);

	   if(root->proxy[0])
         sprintf(sendform, "GET http://%s%s HTTP/1.1\r\n%s\r\n",root->server,ue_word,para);
	   else
         sprintf(sendform, "GET %s HTTP/1.1\r\n%s\r\n",ue_word,para);
	     
//       printf("DEBUG: form %s\n",sendform);

       sprintf(folder, "%s/%s", NEWS_ROOT, root->site);

       if (access(folder,0))
         mak_dirs(folder);


	   if(root->proxy[0])
	     http_conn(root->server,root->port,sendform,root->class[i],root->site,root->proxy,root->proxyport);
	   else
	     http_conn(root->server,root->port,sendform,root->class[i],root->site,NULL,0);
    }
  }
  fclose(fp);
  return 0;
}
