/*------------------------------------------------------*/
/* battr.h     ( YZU WindTopBBS Ver 3.02 )              */
/*------------------------------------------------------*/
/* author : gaod.bbs@bbs.yzu.edu.tw                     */
/* target : Board Attribution                           */
/* create : 2008/10/05                                  */
/*------------------------------------------------------*/

#ifndef _BATTR_H_
#define _BATTR_H_

/* ----------------------------------------------------- */
/* Board Attribution : flags in BRD.battr                */
/* ----------------------------------------------------- */

#define BRD_NOZAP        0x000000001      /* ���i zap */
#define BRD_NOTRAN       0x000000002      /* ����H */
#define BRD_NOCOUNT      0x000000004      /* ���p�峹�o��g�� */
#define BRD_NOSTAT       0x000000008      /* ���ǤJ�������D�έp */
#define BRD_NOVOTE       0x000000010      /* �����G�벼���G�� [sysop] �O */
#define BRD_ANONYMOUS    0x000000020      /* �ΦW�ݪO */
#define BRD_NOFORWARD    0x000000040      /* lkchu.981201: ���i��H */
#define BRD_LOGEMAIL     0x000000080      /* �۰ʪ��[e-mail */
#define BRD_NOBAN        0x000000100      /* ���׫H */
#define BRD_NOLOG        0x000000200      /* �����������H�k */
#define BRD_NOCNTCROSS   0x000000400      /* ������ cross post */
#define BRD_NOREPLY      0x000000800      /* ����^�峹 -- cache.090928: �T��o��,�^���� */
#define BRD_NOLOGREAD    0x000001000      /* �������ݪ��\Ū�v */
#define BRD_CHECKWATER   0x000002000      /* ����������� */
#define BRD_CHANGETITLE  0x000004000      /* �O�D�ק睊�W */
#define BRD_MODIFY       0x000008000      /* �ϥΪ̤��i�ק�峹 */
#define	BRD_PRH		     0x000010000      /* �������ˤ峹 */
#define BRD_PUSHDISCON   0x000020000      /* ���i�PID�s�� */
#define BRD_PUSHTIME     0x000040000      /* ���i�ֳt�s�� */
#define BRD_PUSHSNEER    0x000080000      /* �i�H���N�� */
#define BRD_PUSHDEFINE   0x000100000      /* �i�H�ۭq����ʵ� */
#define	BRD_NOTOTAL	     0x000200000	  /* ���έp�ݪO�ϥά��� */
#define BRD_USIES        0x000400000      /* �[�ݶi�O���� */
#define BRD_BOTTOM	     0x000800000      /* �m���\�� */
#define BRD_VALUE	     0x001000000      /* �u�}�I�ƬݪO */
#define BRD_26 	     	 0x002000000      /* �����D�Ũϥ� */
#define BRD_NOPHONETIC   0x004000000      /* �`���孭��*/
#define BRD_THRESHOLD    0x008000000      /* �o�孭��ݪO */
#define BRD_POSTFIX      0x010000000      /* �O�D�i�ۭq�o�����O */
#define BRD_RSS          0x020000000      /* RSS�ݪO */

/* ----------------------------------------------------- */
/* �U�غX�Ъ�����N�q                                    */
/* ----------------------------------------------------- */

#ifdef _ADMIN_C_

#define NUMATTRS        30

char *battrs[NUMATTRS] = 
{
      "���i Zap",
      "����H",
      "���O���g��",
      "�����������D�έp",
      "�����}���|���G",
      "�ΦW�ݪO",
      "���i��H��K�峹",
      "�۰ʪ��[e-mail",
      "���׫H",
#ifdef  HAVE_DETECT_VIOLAWATE
      "�����������H�k",
#else
      "�����������H�k(�t�Υ\\�ॼ�}��)",
#endif
#ifdef  HAVE_DETECT_CROSSPOST
      "������ cross post",
#else
      "������ cross post(�t�Υ\\�ॼ�}��)",
#endif
      "�ݪO��Ū�T��o��^��",
      "�������ݪ��\\Ū�v",
#ifdef  HAVE_RESIST_WATER
      "�����ݪ��������",
#else
      "�����ݪ��������(�t�Υ\\�ॼ�}��)",
#endif
#ifdef  HAVE_BRDTITLE_CHANGE
      "�O�D�ק睊�W",
#else
      "�O�D�ק睊�W(�t�Υ\\�ॼ�}��)",
#endif
#ifdef  HAVE_USER_MODIFY
      "�ϥΪ̤��i�ק�峹",
#else
      "�ϥΪ̤��i�ק�峹(�t�Υ\\�ॼ�}��)",
#endif
#ifdef  HAVE_RECOMMEND
      "�T����ˤ峹",
#else
      "�T����ˤ峹(�t�Υ\\�ॼ�}��)",
#endif
#ifdef MultiRecommend
      "���i�PID�s��",
      "���i�ֳt�s��",
      "�i�H���N��",
      "�i�H�ۭq����ʵ�",                   
#else
      "���i�PID�s��(�t�Υ\\�ॼ�}��)",
      "���i�ֳt�s��(�t�Υ\\�ॼ�}��)",
      "�i�H���N��(�t�Υ\\�ॼ�}��)",
      "�i�H�ۭq����ʵ�(�t�Υ\\�ॼ�}��)",  
#endif
#ifdef HAVE_COUNT_BOARD
      "�������ݪO��T�έp",
#else
      "�������ݪO��T�έp(�t�Υ\\�ॼ�}��)",
#endif
      "�[�ݶi�O����",
#ifdef HAVE_POST_BOTTOM
      "�峹�m��",
#else
      "�峹�m��(�t�Υ\\�ॼ�}��)",
#endif
      "�u�}�I�ƬݪO",
      "���ϥ�",
#ifdef ANTI_PHONETIC 
      "�T��`����",
#else
      "�T��`����(�t�Υ\\�ॼ�}��)",
#endif      
      "�o�孭��ݪO",
      "�O�D�i�ۭq�o�����O",
      "RSS�ݪO"
};
#endif	/* _ADMIN_C_ */

#endif	/* _BATTR_H_ */

