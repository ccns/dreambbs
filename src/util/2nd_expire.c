/*-------------------------------------------------------*/
/* util/2nd_expire.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : 跳蚤市場佈告刪除程式			 */
/* create : 00/05/05				 	 */
/* update : 						 */
/*-------------------------------------------------------*/
/* author : Ernie.bbs@bbs.cs.nthu.edu.tw		 */
/*-------------------------------------------------------*/
#include "bbs.h"

#define	EXP	(30)
#define FN_GRP                  ".GRP"
#define FN_ITEM                 ".ITEM"

#define PROP_EMPTY              0x0000  // slot is empty, available
#define PROP_G_GROUP            0x0001
#define PROP_G_CANCEL           0x0002
#define PROP_G_OPEN             0x0004
#define PROP_I_SELL             0x0010
#define PROP_I_WANT             0x0020
#define PROP_I_CANCEL           0x0040  // if set, another prog will expire
                                        // this item.
#define PROP_IS_GROUP           (PROP_G_GROUP | PROP_G_CANCEL)
#define PROP_IS_ITEM            (PROP_I_SELL | PROP_I_WANT | PROP_I_CANCEL)
#define PROP_IS_CANCEL          (PROP_G_CANCEL | PROP_I_CANCEL)

typedef struct SLOT             // if is group, only prop/reply/title/fn
{                               // will be used
  time_t chrono;                // time stamp
  int prop;                     // propety of this slot
  int reply;                    // number of replied mail of this item
  char title[30];
  char userid[IDLEN + 1];       // userid for mail reply and owner del check
  char price[10];
  char contact[20];
  char date[6];                 // only 6 bytes, not 9
  char fn[9];                   // dirname or filename of item desc, max9 bytes
} SLOT;

void
expire_grp()
{
  int pos, fd;
  char fgrp[80], fold[80], fnew[80];
  SLOT grp;

  pos = 0;
  sprintf(fgrp, "2nd/%s", FN_GRP);
  sprintf(fnew, "2nd/%s.new", FN_GRP);
  fd = open(fnew, O_CREAT | O_TRUNC, 0600);	// touch a new file
  if(fd > 0) close(fd);

  while(rec_get(fgrp, &grp, sizeof(SLOT), pos) != -1)
  {
    if(grp.prop & PROP_G_CANCEL)
    {
      sprintf(fold, "2nd/%s", grp.fn);
      f_rm(fold);
    }
    else
    {
      rec_add(fnew, &grp, sizeof(SLOT));
    }
    pos++;
  }
  sprintf(fold, "2nd/%s.old", FN_GRP);
  f_cp(fgrp, fold, O_TRUNC);
  f_mv(fnew, fgrp);

  return;
}

void
expire_item()
{
  int pos, pos1, fd;
  char fgrp[80], fitem[80], fname[80], fold[80], fnew[80];
  time_t expire;
  SLOT grp, item;

  expire = time(0) - EXP * 86400;
  pos = pos1 = 0;
  sprintf(fgrp, "2nd/%s", FN_GRP);

  while(rec_get(fgrp, &grp, sizeof(SLOT), pos) != -1)
  {
    sprintf(fitem, "2nd/%s/%s", grp.fn, FN_ITEM);
    sprintf(fnew, "2nd/%s/%s.new", grp.fn, FN_ITEM);

    fd = open(fnew, O_CREAT | O_TRUNC, 0600);	// touch a new file
    if(fd > 0) close(fd);
    fd = 0;

    while(rec_get(fitem, &item, sizeof(SLOT), pos1) != -1)
    {
      if((item.prop & PROP_I_CANCEL) || (item.chrono < expire))
      {
        sprintf(fname, "2nd/%s/%c/%s", grp.fn, item.fn[7], item.fn);
        f_rm(fname);
      }
      else
      {
        fd++;
        rec_add(fnew, &item, sizeof(SLOT));
      }
      pos1++;
    }
    sprintf(fold, "2nd/%s/%s.old", grp.fn, FN_ITEM);
    f_cp(fitem, fold, O_TRUNC);
    f_mv(fnew, fitem);

    grp.reply = fd;
    rec_put(fgrp, &grp, sizeof(SLOT), pos);
    pos++;
  }

  return;
}

int
main()
{
  expire_grp();
  expire_item();

  return 0;
}

