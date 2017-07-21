/* ----------------------------------------------------- */
/*  txt2html.c     ( YZU WindTopBBS Ver 3.XX )		 */
/* ----------------------------------------------------- */
/*  author : visor.bbs@bbs.yzu.edu.tw			 */
/*  target : txt2html					 */
/*  create : 						 */
/*  update :						 */
/* ----------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "getopt.h"

#define CSTRLEN    4096
#define LSTRLEN    4096
#define	MAXLEN	   4096
#define	XPAGE_SIZE 4096
#define	YEA	1
#define	NA	0

static char *spec_symbolic[]=
{
  "<lt",
  ">gt",
  "&amp",
  "\"quot",
  NULL
};


enum ANSIATTR
{
  ATTR_NORMAL,
  ATTR_HIGHLIGHT,       /*high light*/
  ATTR_LOWLIGHT,        /*low  light*/
  ATTR_ITALIC,
  ATTR_UNDER,
  ATTR_BLINK,	        /*閃動*/
  ATTR_UNKNOWN2,        /*快速閃動*/
  ATTR_UNKNOWN3         /*反白*/
};

static int fontctrl=0, fgcolor=37, bgcolor=40, highlight = 0;
static int bgctrl=0;

FILE *filein,*fileout;


char *
transym(char *sstr)
{
  int i,j,used[4] = {0};
  int len,check;
  char *buf,*ps;
  
  for(j=0;sstr[j] != '\0' ;j++)
  {
    for(i=0; spec_symbolic[i] != NULL; i++)
    {
      if(sstr[j]==spec_symbolic[i][0])
      {
        used[i]++;
        break;
      }
    }
  }
  
  
  len = strlen(sstr);
  for(i=0; spec_symbolic[i] != NULL; i++)
    len += (strlen(spec_symbolic[i])+1)*used[i];
  
  ps = buf = (char *)malloc(len+1);
  memset(buf,0,len+1);
  
  for(j=0;sstr[j] != '\0' ;j++)
  {
    for(i=0; spec_symbolic[i] != NULL; i++)
    {
      check = 0;
      if(sstr[j]==spec_symbolic[i][0])
      {
        sprintf(ps,"&%s;",&(spec_symbolic[i][1]));
        ps=ps+strlen(spec_symbolic[i])+1;
        check = 1;
        break;
      }
    }
    if(!check)
      *ps++ = sstr[j];
  }
  return buf;
}

static void 
strip_multicolorword(unsigned char *buf)
{
   unsigned char *ptr;
   unsigned char *des,*colorptr;
   unsigned char colorbuf[32];
   int  state = 0;
   int  precc,cc;
   
   precc = 0;
   colorbuf[0] = '\0';
   
   ptr = des = buf;
   for(;*ptr;ptr++)
   {
     cc = *ptr;
     if(state == 0)
     {
       if(cc == '\033')
       {
         colorptr = colorbuf;
         *colorptr++ = cc;
         ptr++;
         for(;*ptr;ptr++)
         {
           cc = *ptr;
           if((colorptr - colorbuf) < 31)
             *colorptr++ = cc;
           if(cc == '\n')
           {
             colorbuf[0] = '\0';
             break;
           }  
           if(cc == 'm')
           {
             cc = *++ptr;
             break;
           }
         }
         *colorptr = '\0';
       }
       
       if(colorbuf[0])
       {
         memcpy(des,colorbuf,strlen(colorbuf));
         des+=strlen(colorbuf);
         colorbuf[0] = '\0';
       }       

       if(cc >= 0xA1 && cc <= 0xF9)
       {
         state = 1;
         precc = cc;
       }
       else if(cc < 0x80)
         *des++ = cc;
     }
     else if(state == 1)
     {
       if(cc == '\033')
       {
         colorptr = colorbuf;
         *colorptr++ = cc;
         ptr++;
         for(;*ptr;ptr++)
         {
           cc = *ptr;
           if((colorptr - colorbuf) < 31)
             *colorptr++ = cc;
           if(cc == '\n')
           {
             colorbuf[0] = '\0';
             break;
           }  
           if(cc == 'm')
           {
             cc = *++ptr;
             break;
           }
         }
         *colorptr = '\0';
       }
       
       if(cc == '\n')
       {
         *des++=cc;
       }
       else if((cc >= 0xA1 && cc <= 0xFE) || (cc >= 0x40 && cc <= 0x7E))
       {
         *des++=precc;
         *des++=cc;
         if(colorbuf[0])
         {
           memcpy(des,colorbuf,strlen(colorbuf));
           des+=strlen(colorbuf);
           colorbuf[0] = '\0';
         }
       }
       state = 0;
     }
   }
   if(colorbuf[0])
   {
     strcpy(des,colorbuf);
     des+=strlen(colorbuf)-1;
     colorbuf[0] = '\0';
   }
   *des = '\0';
   return;
}


static char * 
escapestr(char*dstr,char *sstr)
{
  int i,j,normalchar;
  char *ps;
  int check;

  enum ANSIATTR attr=ATTR_NORMAL;
  char codebuf[CSTRLEN], *pc;

  ps=dstr;
  normalchar=YEA;

  strip_multicolorword(sstr);

  for(j=0; sstr[j] != '\0'; j++)
  {
    if(sstr[j] =='\033')
    {
      check = 0;
      if(sstr[j+1]=='[')
      {
        if(attr==ATTR_ITALIC)
        {
          sprintf(ps,"</I>");
          ps+=4;
        }
        if(attr==ATTR_UNDER)
        {
          sprintf(ps,"</U>");
          ps+=4;
        }
        if(attr==ATTR_BLINK)
        {
          sprintf(ps,"</BLINK>");
          ps+=8;
        }
        j+=2;
        i=0;
        if(sstr[j] == ';')
        {
          codebuf[i]='0';
          i++;
          j--;
        }
        while(sstr[j+i] != 'm')
        {
          codebuf[i]=sstr[j+i];
          i++;
        }
        j+=i;
        codebuf[i]='\0';
        if(i==0)
          attr=ATTR_NORMAL;
        pc=strtok(codebuf,";");
        if(pc==NULL)
        {
          if(bgctrl)
          {
            sprintf(ps,"</FONT>");
            ps+=7;
          }
          if(fontctrl)
          {
            sprintf(ps,"</FONT>");
            ps+=7;
          }
          attr= 0;	//ANSIATTR(0);
          fgcolor=37;
          highlight=0;
          bgcolor=40;
          fontctrl=YEA;
          bgctrl=YEA;
          sprintf(ps,"<FONT class=col%d%d>",highlight,fgcolor);
          ps+=19;
          sprintf(ps,"<FONT class=col0%d>",bgcolor);
          ps+=19;
        }
        while(pc!=NULL)
        {
          i=atoi(pc);
          if(i>=30 && i <=37)
          {
            if(fontctrl)
            {
              sprintf(ps,"</FONT>");
              ps+=7;
              if(bgctrl)
              {
                 sprintf(ps,"</FONT><FONT class=col0%d>",bgcolor);
                 ps+=26;
//                 sprintf(ps,"<FONT class=col0%d>",bgcolor);
//                 ps+=19;
              }
            }
            check=1;
            fgcolor=i;
            sprintf(ps,"<FONT class=col%d%d>",highlight,fgcolor);
            ps+=19;
            fontctrl=YEA;
          }
          else if(i>=40 && i <=47)
          {
            if(bgctrl)
            {
              sprintf(ps,"</FONT>");
              ps+=7;
              if(fontctrl)
              {
                sprintf(ps,"</FONT><FONT class=col%d%d>",highlight,fgcolor);
                ps+=26;
//                sprintf(ps,"<FONT class=col%d%d>",highlight,fgcolor);
//                ps+=19;
              }
            }
            bgcolor=i;
            sprintf(ps,"<FONT class=col0%d>",bgcolor);
            ps+=19;
            bgctrl=YEA;
          }
          else if(i==ATTR_ITALIC || i==ATTR_UNDER)
          {
            attr= i;	//ANSIATTR(i);
          }
          else if(i==ATTR_NORMAL || *pc == '\0')
          {
            if(bgctrl)
            {
              sprintf(ps,"</FONT>");
              ps+=7;
            }
            if(fontctrl)
            {
              sprintf(ps,"</FONT>");
              ps+=7;
            }
            attr= i;	//ANSIATTR(i);
            fgcolor=37;
            highlight=0;
            bgcolor=40;
            fontctrl=YEA;
            bgctrl=YEA;
            check=1;
            sprintf(ps,"<FONT class=col%d%d>",highlight,fgcolor);
            ps+=19;
            sprintf(ps,"<FONT class=col0%d>",bgcolor);
            ps+=19;
          }
          else if(i==ATTR_HIGHLIGHT)
          {
            check=0;
            highlight=1;
          }
          pc=strtok(NULL,";");
        }
        if(check == 0 && i==ATTR_HIGHLIGHT)
        {
           if(fontctrl)
           {
             sprintf(ps,"</FONT>");
             ps+=7;
             if(bgctrl)
             {
               sprintf(ps,"</FONT><FONT class=col0%d>",bgcolor);
               ps+=26;
//               sprintf(ps,"<FONT class=col0%d>",bgcolor);
//               ps+=19;
             }
           }
           sprintf(ps,"<FONT class=col%d%d>",highlight,fgcolor);
           ps+=19;
           fontctrl=YEA;
        }

        if(attr==ATTR_ITALIC)
        {
          sprintf(ps,"<I>");
          ps+=3;
        }
        if(attr==ATTR_UNDER)
        {
          sprintf(ps,"<U>");
          ps+=3;
        }
        if(attr==ATTR_BLINK)
        {
          sprintf(ps,"<BLINK>");
          ps+=7;
        }

        continue;
      }
    }

    for(i=0; spec_symbolic[i] != NULL; i++)
    {
      if(sstr[j]==spec_symbolic[i][0])
      {
        sprintf(ps,"&%s;",&(spec_symbolic[i][1]));
        ps=ps+strlen(spec_symbolic[i])+1;
        normalchar=NA;
        break;
      }
    }

    if(normalchar)
    {
      *ps=sstr[j];
      if(*ps == '\033')
        *ps='*';
      ps++;
    }
    else
      normalchar=YEA;
  }
  *ps='\0';
  return dstr;
}

static char *
add_str(dst,src,len,page)
  char *dst;
  char *src;
  int *len;
  int *page;
{
  int slen;
  
  char *code;
  code = dst;
  
  slen = strlen(src);

  if(slen + *len >= *page * XPAGE_SIZE)
  {
    *page += 1;
    code = (char *)realloc(code,(*page)*XPAGE_SIZE);
  }
  strcpy(code+*len,src);
  *len += slen;
  
  return code;
}

char *
prebuf(char *buf)
{
   char tmpbuf[LSTRLEN];
   
   strcpy(tmpbuf,"\033[;32m");
   if(!strncmp(buf,"> ",2))
   {
     strcpy(tmpbuf+6,buf);
     strcat(tmpbuf,"\033[m");
     strcpy(buf,tmpbuf);
   }
   return buf;
}

char *
txt2html(FILE *fp)
{
  char buf[LSTRLEN],encodebuf[LSTRLEN],line[MAXLEN],*code;
  int len,page,usehead,i;
  char *head[] = {"作者", "標題", "時間", "路徑", "來源"};
  char *headvalue,*ptr,*pbrd,board[128];

  pbrd = NULL;
  len = 0;
  page = 1;
  code = (char *)malloc(XPAGE_SIZE);
  memset(code,0,XPAGE_SIZE);

  fontctrl=0;
  bgctrl=0;
  fgcolor=37;
  bgcolor=40;

  strcpy(line,"<table width='800' border='0' cellspacing='0' cellpadding='0'>\n");
  code = add_str(code,line,&len,&page);

  i = 0;
  usehead = 0;
  while( i<5 && (ptr = fgets(buf,sizeof(buf),fp)) != NULL)
  {
    if(!usehead && (strstr(buf,"作者:") || strstr(buf,"發信人:")))
      usehead = 1;
      
    if(i == 0 && usehead && (strstr(buf,"看板:") || strstr(buf,"站內:")))
    {
      pbrd = strrchr(buf,':');
      *pbrd = '\0';
      strcpy(board,pbrd + 2);         
      pbrd -= 5;
      *pbrd++ = '\0';
    }

    if(i==0 && usehead == 0)
    {
      strcpy(line,"<TR><TD COLSPAN=4><PRE>");
      code = add_str(code,line,&len,&page);
      strcpy(line,escapestr(encodebuf,buf));
      code = add_str(code,line,&len,&page);
      break;
    }
    headvalue=strchr(buf,':');
    if( headvalue==NULL )
    {
      strcpy(line,"<TR><TD COLSPAN=4><PRE><HR><br>");
      code = add_str(code,line,&len,&page);
      strcpy(line,escapestr(encodebuf,buf));
      code = add_str(code,line,&len,&page);
      break;
    }
    headvalue[0] = '\0';
    headvalue = escapestr(encodebuf,headvalue+2);
    if(i == 0 && pbrd)
      sprintf(line, "<TR><TD ALIGN=MIDDLE class=col047><font class=col034>%s</TD><TD class=col044><font class=col037>&nbsp;%s</TD><TD ALIGN=MIDDLE class=col047><font class=col034>%s</TD><TD class=col044><font class=col037>&nbsp;%s</TD></TR>\n",head[i],headvalue,pbrd,board);
    else
      sprintf(line, "<TR><TD ALIGN=MIDDLE class=col047><font class=col034>%s</TD><TD COLSPAN=3 class=col044><font class=col037>&nbsp;%s</TD></TR>\n",head[i],headvalue);
    code = add_str(code,line,&len,&page);
    i++;
  }
  if(!ptr)
  {
    strcpy(line,"<TR><TD COLSPAN=4><PRE>");
    code = add_str(code,line,&len,&page);
  }
  
  strcpy(buf,"\033[m");
  strcpy(line,escapestr(encodebuf,buf));
  code = add_str(code,line,&len,&page);

  while(fgets(buf,sizeof(buf),fp) != NULL)
  {
    strcpy(line,escapestr(encodebuf,prebuf(buf)));
    code = add_str(code,line,&len,&page);
  }
  strcpy(line,"<br></PRE></TD></TR></table>\n");
  code = add_str(code,line,&len,&page);

  return code;
}

void show_style()
{
	fprintf(fileout,"<style type=\"text/css\">" \
"PRE             {font-size: 15pt; line-height: 15pt; font-weight: lighter; background-color: 000000; COLOR: c0c0c0;}" \
"TD              {font-size: 15pt; line-height: 15pt; font-weight: lighter;}" \
"FONT.col030     {COLOR: 000000;}" \
"FONT.col031     {COLOR: a00000;}" \
"FONT.col032     {COLOR: 00a000;}" \
"FONT.col033     {COLOR: a0a000;}" \
"FONT.col034     {COLOR: 0000a0;}" \
"FONT.col035     {COLOR: a000a0;}" \
"FONT.col036     {COLOR: 00a0a0;}" \
"FONT.col037     {COLOR: c0c0c0;}" \
"FONT.col040     {background-color: 000000;}" \
"FONT.col041     {background-color: a00000;}" \
"FONT.col042     {background-color: 00a000;}" \
"FONT.col043     {background-color: a0a000;}" \
"FONT.col044     {background-color: 0000a0;}" \
"FONT.col045     {background-color: a000a0;}" \
"FONT.col046     {background-color: 00a0a0;}" \
"FONT.col047     {background-color: c0c0c0;}" \
"FONT.col130     {COLOR: 606060;}" \
"FONT.col131     {COLOR: ff0000;}" \
"FONT.col132     {COLOR: 00ff00;}" \
"FONT.col133     {COLOR: ffff00;}" \
"FONT.col134     {COLOR: 0000ff;}" \
"FONT.col135     {COLOR: ff00ff;}" \
"FONT.col136     {COLOR: 00ffff;}" \
"FONT.col137     {COLOR: e0e0e0;}" \
"TD.col044       {background-color: 0000a0;}" \
"TD.col047       {background-color: c0c0c0;}" \
"HR		{COLOR: 00a0a0; height: 1pt;}" \
"</style>\n");
}

int main (int argc, char * argv[]) 
{
    char *ptr,*pn;
    char infile[256],outfile[256];
    int c;

    
   struct option      longopts[]   = {
      { "version",  		0, 0, 'V' },
      { "std-input",   		0, 0, 'I' },
      { "std-output",     	0, 0, 'O' },
      { "file-input",   	1, 0, 'i' },
      { "file-output",   	1, 0, 'o' },
      { "help",     		0, 0, 'h' },
      { 0,          	        0, 0, 0  }
   };

   infile[0] = '\0';
   outfile[0] = '\0';
   filein = stdin;
   fileout = stdout;

   pn = strrchr (argv[0], '/');
   if (pn) pn ++; else pn = argv[0];

   while ((c = getopt_long( argc, argv,
                            "VIOi:o:h", longopts, NULL )) != EOF)
      switch (c) {
      case 'V': exit(1);                                  break;
      case 'I': filein = stdin;                           break;
      case 'O': fileout = stdout;                         break;
      case 'i': printf("%s\n",optarg);strcpy(infile,optarg);               	  break;
      case 'o': strcpy(outfile,optarg);            break;
      case 'h': exit(1);              break;
      default :
        fprintf (stderr, "Try `%s --help' for more information.\n\r", pn);
        exit(1);
        break;

      }
      
    if(infile[0])
    {
      filein = fopen(infile,"r");
      if(!filein)
      {
        fprintf(stderr,"fopen: %s\n", infile);
        exit(1);
      }
    }
    if(outfile[0])
    {
      fileout = fopen(outfile,"w");
      if(!fileout)
      {
        fprintf(stderr,"fopen: %s\n", outfile);
        exit(1);
      }
    }  
      
    
    show_style();
    ptr = txt2html(filein);
    fprintf(fileout,ptr);
    free(ptr);

    fclose(filein);
    fclose(fileout);


}


