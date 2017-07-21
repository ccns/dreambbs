/*-------------------------------------------------------*/
/* banmail.c	( YZU_CSE WindTop BBS )			 */
/*-------------------------------------------------------*/
/* author : visor.bbs@bbs.yzu.edu.tw                     */
/* target : ban_mail_list routines		 	 */
/* create : 99/12/20				 	 */
/* update : 99/12/31				 	 */
/*-------------------------------------------------------*/


#undef	_MODES_C_
#include "bbs.h"

extern XZ xz[];

/* ----------------------------------------------------- */
/* �׫H�C��G��榡�ާ@�ɭ��y�z				 */
/* ----------------------------------------------------- */


static int banmail_add();


static void
banmail_item(num, ban)
  int num;
  BANMAIL *ban;
{
  time_t now;
  char modes[7];
  sprintf(modes,"%c%c%c%c%c%c",(ban->mode&FW_OWNER)?'1':'0',(ban->mode&FW_TITLE)?'1':'0',
          (ban->mode&FW_TIME)?'1':'0',(ban->mode&FW_PATH)?'1':'0',
          (ban->mode&FW_ORIGIN)?'1':'0',(ban->mode&FW_CANCEL)?'1':'0');

  now = ((ban->time - time(0) + BANMAIL_EXPIRE*86400)/3600);
  prints("%6d  %6d %6d %s  %-48.48s\n", num, ban->usage, now < 0 ? 0 : now,modes,ban->data);
}

static int
banmail_body(xo)
  XO *xo;
{
  BANMAIL *banmail = NULL;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    if (vans("�n�s�W��ƶ�(Y/N)�H[N] ") == 'y')
      return banmail_add(xo);
    return XO_QUIT;
  }

  banmail = (BANMAIL *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;

  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    banmail_item(++num, banmail++);
  } while (num < max);
  clrtobot();

  return XO_NONE;
}


static int
banmail_head(xo)
  XO *xo;
{
  vs_head("�׫H�C��", str_site);
  outs("\
  [��]���} ^P)�s�W c)�ק� d)�R�� S)���� [h]elp\n\
\033[30;47m  �s��  �ϥβv ��s�� ��  ��  ��  �H  ��  �e                                  \033[m");
  return banmail_body(xo);
}


static int
banmail_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(BANMAIL));
  return banmail_body(xo);
}


static int
banmail_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(BANMAIL));
  return banmail_head(xo);
}

static int
banmail_sync(xo)
  XO *xo;
{
  char *fpath;
  int fd, size=0;
  struct stat st;

  fpath = xo->dir;

  if ((fd = open(fpath, O_RDWR, 0600)) < 0)
    return XO_NONE;

  outz("�� ��ƾ�z�]�֤��A�еy�� \033[5m...\033[m");
  refresh();

  if (!fstat(fd, &st) && (size = st.st_size) > 0)
  {
    BANMAIL *pbase, *phead, *ptail;

    pbase = phead = (BANMAIL *) malloc(size);
    size = read(fd, pbase, size);
    if (size >= sizeof(BANMAIL))
    {
      ptail = (BANMAIL *) ((char *) pbase + size);

      size = (char *) ptail - (char *) pbase;
      if (size > 0)
      {
        if (size > sizeof(BANMAIL))
          xsort(pbase, size / sizeof(BANMAIL), sizeof(BANMAIL), (void *)str_cmp);

        lseek(fd, 0, SEEK_SET);
        write(fd, pbase, size);
        ftruncate(fd, size);
      }
    }
    free(pbase);
  }
  close(fd);
  if (size <= 0)
    unlink(fpath);
  return XO_INIT;
}


static int
banmail_edit(banmail,echo)
  BANMAIL *banmail;
  int echo;
{
  int change = 0;
  char modes[8],buf[64];

  if(echo == DOECHO)
    memset(banmail, 0, sizeof(BANMAIL));

  sprintf(modes,"%c%c%c%c%c%c",(banmail->mode&FW_OWNER)?'1':'0',(banmail->mode&FW_TITLE)?'1':'0',
          (banmail->mode&FW_TIME)?'1':'0',(banmail->mode&FW_PATH)?'1':'0',
          (banmail->mode&FW_ORIGIN)?'1':'0',(banmail->mode&FW_CANCEL)?'1':'0');  

  if(vget(b_lines, 0, "�׫H�C��G", banmail->data, sizeof(banmail->data), echo))
    change++;
  sprintf(buf,"�׫H�Ҧ��G(�@��,���D,�ɶ�,���|,�ӷ�,�s�u��H)[%s]",modes);
  if(vget(b_lines,0,buf,modes,8,GCARRY))
  {
    banmail->mode=(modes[0]!='0')?FW_OWNER:0;
    banmail->mode|=(modes[1]!='0')?FW_TITLE:0;
    banmail->mode|=(modes[2]!='0')?FW_TIME:0;
    banmail->mode|=(modes[3]!='0')?FW_PATH:0;
    banmail->mode|=(modes[4]!='0')?FW_ORIGIN:0;
    banmail->mode|=(modes[5]!='0')?FW_CANCEL:0;
    change++;
  }
  
  if(change)  
    return 1;
  else
    return 0;
}


static int
banmail_add(xo)
  XO *xo;
{
    BANMAIL banmail;

    if(banmail_edit(&banmail,DOECHO))
    {
      banmail.time = time(0);
      rec_add(xo->dir, &banmail, sizeof(BANMAIL));
      xo->pos = XO_TAIL /* xo->max */ ;
      xo_load(xo, sizeof(BANMAIL));
    }
  return banmail_head(xo);
}

static int
banmail_delete(xo)
  XO *xo;
{

  if (vans(msg_del_ny) == 'y')
  {
    if (!rec_del(xo->dir, sizeof(BANMAIL), xo->pos, NULL, NULL))
    {
      return banmail_load(xo);
    }
  }
  return XO_FOOT;
}


static int
banmail_change(xo)
  XO *xo;
{
  BANMAIL *banmail, mate;
  int pos, cur;
  
  pos = xo->pos;
  cur = pos - xo->top;
  banmail = (BANMAIL *) xo_pool + cur;

  mate = *banmail;
  banmail_edit(banmail, GCARRY);
  if (memcmp(banmail, &mate, sizeof(BANMAIL)))
  {
    banmail->time = time(0);
    rec_put(xo->dir, banmail, sizeof(BANMAIL), pos);
    move(3 + cur, 0);
    banmail_item(++pos, banmail);
  }

  return XO_FOOT;
}

static int
banmail_help(xo)
  XO *xo;
{
  film_out(FILM_BANMAIL, -1);
  return banmail_head(xo);
}


KeyFunc banmail_cb[] =
{
  {XO_INIT, banmail_init},
  {XO_LOAD, banmail_load},
  {XO_HEAD, banmail_head},
  {XO_BODY, banmail_body},

  {Ctrl('P'), banmail_add},
  {'S', banmail_sync},
  {'r', banmail_change},
  {'c', banmail_change},
  {'s', banmail_init},
  {'d', banmail_delete},
  {'h', banmail_help}
};


int
BanMail()
{
  XO *xo;
  char fpath[64];
  sprintf(fpath, FN_ETC_BANMAIL_ACL);
  xz[XZ_BANMAIL - XO_ZONE].xo = xo = xo_new(fpath);
  xz[XZ_BANMAIL - XO_ZONE].cb = banmail_cb;
  xover(XZ_BANMAIL);
  fwshm_load();
  free(xo);
  return 0;
}

void
post_mail()
{
   XO *xx;
   char fpath[64];

   sprintf(fpath, "brd/%s/banmail.acl", currboard);
   xz[XZ_BANMAIL - XO_ZONE].xo = xx = xo_new(fpath);
   xz[XZ_BANMAIL - XO_ZONE].cb = banmail_cb;
   xover(XZ_BANMAIL);
   free(xx);
}

