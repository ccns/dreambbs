/*-------------------------------------------------------*/
/* util/showACCT.c      ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : ��ܨϥΪ̸��                               */
/* create : 01/07/16                                     */
/* update :                                              */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#undef  SHOW_PASSWORD           /* ��ܱK�X (�ܯӮɶ�) */


#ifdef SHOW_PASSWORD

#define GUESS_LEN       3               /* �u���T�X(�t)�H�U���K�X�զX (�̦h�O PSWDLEN) */

static inline void
showpasswd(passwd)
  char *passwd;
{
  int i;
  char guess[PSWDLEN + 1];

  /* �L�׬O���� encrypt ����k�A���w�q ' ' �}�l try �� 0x7f */

  memset(guess, 0, sizeof(guess));
  guess[0] = ' ';

  while (1)
  {
    guess[0]++;

    /* �i�� */
    for (i = 0; i < GUESS_LEN ;i++)
    {
      if (guess[i] < 0x7f)
        break;

      guess[i] = ' ';
      guess[i + 1]++;
    }

    if (!chkpasswd(passwd, guess))
    {
      printf("�K�X: %s \n", guess);
      break;
    }

    if (i == GUESS_LEN)
    {
      printf("�K�X���׶W�L %d ��\n", GUESS_LEN);
      break;
    }
  }
}
#endif

#if 0
static char *
_bitmsg(str, level)
  char *str;
  int level;
{
  int cc;
  int num;
  static char msg[33];

  num = 0;
  while (cc = *str)
  {
    msg[num] = (level & 1) ? cc : '-';
    level >>= 1;
    str++;
    num++;
  }
  msg[num] = 0;

  return msg;
}
#endif

static inline void
showACCT(acct)
  ACCT *acct;
{
  char /*msg1[40], msg2[40],*/ msg3[40], msg4[40], msg5[40], msg6[40];

  strcpy(msg3, Btime(&(acct->firstlogin)));
  strcpy(msg4, Btime(&(acct->lastlogin)));
  strcpy(msg5, Btime(&(acct->tcheck)));
  strcpy(msg6, Btime(&(acct->tvalid)));

  printf("> ------------------------------------------------------------------------------------------ \n"
    "�s��: %-15d [ID]: %-15s �m�W: %-15s �ʺ�: %-15s \n"
//    "�v��: %-37s �]�w: %-37s \n"
//    "ñ�W: %-37d �ʧO: %-15.2s \n"
//    "�ȹ�: %-15d ����: %-15d �ͤ�: %02d/%02d/%02d \n"
    "�W��: %-15d �峹: %-15d �o�H: %-15d \n"
    "����: %-37s �W��: %-30s \n"
    "�ˬd: %-37s �q�L: %-30s \n"
    "�n�J: %-30s \n"
    "�H�c: %-60s \n",
    acct->userno, acct->userid, acct->realname, acct->username,
//    msg1, msg2,
//    acct->signature, /*"�H���" + (acct->sex << 1),*/
//    acct->money, acct->gold, acct->year, acct->month, acct->day,
    acct->numlogins, acct->numposts, acct->numemail,
    msg3, msg4,
    msg5, msg6,
    acct->lasthost,
    acct->email);

#ifdef SHOW_PASSWORD
  showpasswd(acct->passwd);
#endif
}


int
main(argc, argv)
  int argc;
  char **argv;
{
  int i;

  if (argc < 2)
  {
    printf("Usage: %s UserID1 [UserID2] ...\n", argv[0]);
    return -1;
  }

  chdir(BBSHOME);

  for (i = 1; i < argc; i++)
  {
    ACCT acct;
    char fpath[64];

    usr_fpath(fpath, argv[i], FN_ACCT);
    if (rec_get(fpath, &acct, sizeof(ACCT), 0) < 0)
    {
      printf("%s: read error (maybe no such id?)\n", argv[i]);
      continue;
    }

    showACCT(&acct);
  }
  printf("> ------------------------------------------------------------------------------------------ \n");

  return 0;
}
