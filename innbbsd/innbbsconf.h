/*-------------------------------------------------------*/
/* innbbsconf.h ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : innbbsd configurable settings                */
/* create : 95/04/27                                     */
/* modify :   /  /                                       */
/* author : skhuang@csie.nctu.edu.tw                     */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#ifndef INNBBSCONF_H
#define INNBBSCONF_H

#include "bbs.h"

#include <sys/wait.h>


/* ----------------------------------------------------- */
/* �@��պA                                              */
/* ----------------------------------------------------- */

#define VERSION         "0.8-MapleBBS"          /* �����ŧi */

#define MYBBSID         BBSNAME                 /* Path: �έ^�寸�W */

#define LOGFILE         FN_INNBBS_LOG           /* �O���ɸ��| */

#define INNBBS_PORT     (7777)

/* ----------------------------------------------------- */
/* innbbsd ���]�w                                        */
/* ----------------------------------------------------- */

    /* --------------------------------------------------- */
    /* channel ���]�w                                      */
    /* --------------------------------------------------- */

#define MAXCLIENT       20              /* Maximum number of connections accepted by innbbsd */

#define ChannelSize     4096
#define ReadSize        4096

    /* --------------------------------------------------- */
    /* rec_article ���]�w                                  */
    /* --------------------------------------------------- */

#define NoCeM                           /* No See Them �׫H���� */

#undef  KEEP_CANCEL                     /* �O�d cancel ���峹�� deleted �O */


/* ----------------------------------------------------- */
/* bbslink ���]�w                                        */
/* ----------------------------------------------------- */

#define MAX_ARTS        100             /* �C�� newsgroup �@���̦h���X�ʫH */

#define BBSLINK_EXPIRE  3600            /* bbslink �Y����L�[�A�N���� kill ��(��) */


/* ----------------------------------------------------- */
/* History ��H�O�����@                                  */
/* ----------------------------------------------------- */

#define EXPIREDAYS      5               /* ��H�O���O�d�Ѽ� */
#define HIS_MAINT_HOUR  4               /* time to maintain history database */
#define HIS_MAINT_MIN   30              /* time to maintain history database */

#endif  /* INNBBSCONF_H */