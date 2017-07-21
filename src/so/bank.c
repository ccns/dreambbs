/*-------------------------------------------------------*/
/* so/bank.c                                             */
/*-------------------------------------------------------*/
/* author : shiun.bbs@ccca.mksh.phc.edu.tw               */
/* modify : cat.bbs@ccca.mksh.phc.edu.tw                 */
/* update : cache.bbs@bbs.ee.ncku.edu.tw                 */
/* create : unknown                                      */
/* update : 09/10/11                                     */
/*-------------------------------------------------------*/
/* syntax :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"
/*
void log_bank(mode, a, b, who)
  int mode; 
  int a;
  int b;
  char *who;
{
  time_t now;
  char c_time[25], c_buf[100]={};

  now = time(0);
  strncpy(c_time, ctime(&now), 24);
  c_time[24] = '\0';

  if(mode == 1)
    sprintf(c_buf, "%s %s �u�}�I��(%d)->�ڹ�(%d)\n", c_time, cuser.userid, a, b);
  else if(mode == 2)
    sprintf(c_buf, "%s %s �ڹ�(%d)->�u�}�I��(%d)\n", c_time, cuser.userid, a, b);
  else if(mode == 3)
    sprintf(c_buf, "%s %s �ڹ�(%d)->�Ѳ�(%d)\n", c_time, cuser.userid, a, b);
  else if(mode == 4)
    {
    sprintf(c_buf, "%s %s �״�(%d)-> %s (%d)\n", c_time, cuser.userid, a, *who, b);
    }
  else if(mode == 5)
    sprintf(c_buf, "%s %s �פJ�¹ڹ�(%d)\n", c_time, cuser.userid, a);
    
  f_cat(FN_BANK, c_buf);
}
*/

int point1_money()
{
   int num;
   char buf[10];
   int money;
   ACCT acct;

     if(acct_load(&acct, cuser.userid) >= 0)
       money = acct.money;    
     else
       {
         pmsg2("�d�L�z���b���T..."); 
         return 0;
       }
   
   clearange(0, 22);
   vs_bar("�ڹ��ഫ");
   
   move(2,0);
   prints("�A�����W�� %9d �ڹ�\n\n           %9d �u�}�I��"
           ,acct.money, acct.point1);
   if(acct.point1 < 1)
   {
      pmsg2("�u�}�I�Ƥ���");
      return 0;
   }
      
   vget(8, 0, "�n�ഫ�h���u�}�I�ơH", buf, 8, DOECHO);
   if((num = atoi(buf)) <= 0)
     return 0;
   else
   {
     double temp = num*4096 + acct.money;

     if(temp > INT_MAX)
     {
       pmsg2("�ഫ��ڹ��W�L�W���I");
       return 0;
     }

   temp = (int)temp;
     
   if(acct_load(&acct, cuser.userid) >= 0)
   {
     acct.money = temp;
     acct.point1 -= num;      
   }

   acct_save(&acct);

   time_t now;
   char c_time[25], c_buf[100]={};
   now = time(0);
   strncpy(c_time, ctime(&now), 24);
   c_time[24] = '\0';
   sprintf(c_buf, "%s %s �u�}�I��(%d)->�ڹ�(%d)\n", c_time, cuser.userid, num, temp);
   f_cat(FN_BANK, c_buf);

   }
   pmsg2("�ഫ����");
   return 0;
}


int
TransferAccount()
{
  ACCT acct, selfacct;
  char buf[128];
  time_t now;
  HDR xhdr;
  FILE *fp;
  char folder[128],date[9],fpath[128];
  char userid[13];
  char str[128];
  int selfmoney, pay;
  double temp;
    
  clearange(0, 22);
  vs_bar("�״�");

  move (9, 8);
  prints("\033[1;33m��b�����W�w�G \033[36m��. �@���̤֭n�� 100 �ڹ�(�|�e)�C\n\
                       ��. ������O 10 %%�C\033[m");
                       
  if (acct_get("�n�׵��֡G",&acct)<1)
    return 0;
  if(acct.userno == cuser.userno)
  {
    pmsg2("�����ۤv����աI");
    return 0;
  }
  strcpy(userid,acct.userid);
      
  clearange(1,21);
  move(3,0);
  
     if(acct_load(&selfacct, cuser.userid) >= 0)
       selfmoney = selfacct.money;    
     else
       {
         pmsg2("�d�L�z���b���T..."); 
         return 0;
       }
  
  prints("�A�ۤv�����W�٦� %9d �ڹ��C\n",selfacct.money);
  prints("\n%-12s�h�� %9d �ڹ��C",userid, acct.money); 
  
  if(!vget(7,0,"�A�n�״ڦh�ֹڹ��G",buf,10,DOECHO))
    return 0;

  temp = ((int)atoi(buf) + acct.money);

  if((int)atoi(buf) < 100)
  {
    pmsg2("�״ڪ��B���o�C�� 100 ��");
    return 0;
  }
  else if ((int)atoi(buf) > selfacct.money)
  {
    pmsg2("�״ڪ��B�W�L��ץX���W��");
    return 0;       
  }
  else if (temp>INT_MAX)
  {
    pmsg2("�״ڪ��B�W�L���౵�����W��");
    return 0;    
  }
   
  pay = (int)(atoi(buf)*1.1);
    
  move(9,0);
  prints("���� %d ���ڹ�(�|�e)�A��ڤ�I %d �ڹ�(�|��)", (int)atoi(buf), pay);

  move(11,0);
  clrtobot();
      
  if (!vget(b_lines, 0, "�״ڲz�ѡG", str, 60, DOECHO))
    return 0;  

  if(vans("�T�w�n���L�ܡH [Y/n]") != 'n')
  {

    now = time(0);
    str_stamp(date, &now);
        
    usr_fpath(folder, userid, FN_DIR);
    fp = fdopen(hdr_stamp(folder, 0, &xhdr, fpath),"w");      
    strcpy(xhdr.owner, cuser.userid);
    strcpy(xhdr.nick, cuser.username);
    sprintf(xhdr.title, "�״ڳq��");
    strcpy(xhdr.date, date);
       
    fprintf(fp,"�@��: %s (%s)\n",cuser.userid,cuser.username);
    fprintf(fp,"���D: �״ڳq��\n");
    fprintf(fp,"�ɶ�: %s\n",date);
    fprintf(fp,"\n\n%s �e���A \033[1;31m%d\033[m �ڹ��A�Я���~\n", cuser.userid, (int)atoi(buf));
    fprintf(fp,"\n\n�״ڲz�ѡG%s\n",str);
    fclose(fp);
    rec_add(folder, &xhdr, sizeof(HDR));
        
    acct.money += (int)atoi(buf);
    selfacct.money -= pay;
    acct_save(&acct);
    acct_save(&selfacct);
      
    time_t now;
    char c_time[25], c_buf[100]={};
    now = time(0);
    strncpy(c_time, ctime(&now), 24);
    c_time[24] = '\0';
    sprintf(c_buf, "%s %s �״�(%d)-> %s (%d)\n", c_time, cuser.userid, pay, userid, (int)atoi(buf));
    f_cat(FN_BANK, c_buf);

  }
  
  pmsg2("�������");
  return 0;
}

/* 100618.cache: �¹ڹ��ഫ */
#define FN_MONEY		".MONEY"		/* PostRecommendHistory */

typedef struct
{
  int money;			/* �ڹ� */
  int save;			    /* �s�� */
  int request;			/* �p���I�� */
}	MONEY;

/*
int
money_back(void)
{
pmsg2("�����ʤw�I��");
return 0;
}
*/

	int
money_back(void)
{
  ACCT acct;
  char buf[128];
  char fpath[80];
  int fd;  
  double m1 = 0;
  double m2 = 0;
  double m3 = 0;  
  MONEY oldwealth;

  //if (acct_get("�n�e���I�q���ơG", &acct) < 1)
  //  return 0;
  //clrtobot();

  if (acct_get("�п�JID�G", &acct) < 1)
  {
	pmsg2("�d�L�� ID ���b���T...");
    return 0;
  }
  {
    clrtobot();
    usr_fpath(fpath,acct.userid,FN_MONEY);

	//Ū��MONEY 
    if ((fd = open(fpath, O_RDONLY)) < 0)
    { 
      pmsg2("�d�L�� ID ���¹ڹ����..."); 
      return 0;        
    }

    read(fd, &oldwealth, sizeof(MONEY));
          
    m1 = oldwealth.money;
    m2 = oldwealth.save;
    m3 = oldwealth.request;

    m1 = ((m1+m2)/2) - 1;  //�Цۦ�ק� 

    if (m1 >= INT_MAX )
       m1 = INT_MAX;
    if (m2 >= INT_MAX )	
       m2 = INT_MAX;

    //���F��K�ҥH�S���屼�°O��, ���i��~�� 
    //if (acct.money > 65535)
    //{ 
    //  pmsg2("���ŦX�פJ���..."); 
    //  return 0;        
    //}
	
    //if (acct.money < 10)
    //{
	//pmsg2("���ŦX�פJ���...");
    //return 0;
	//}

    m1 = (int)m1;

    acct.money = m1;
    acct_save(&acct);

    time_t now;
    char c_time[25], c_buf[100]={};
    now = time(0);
    strncpy(c_time, ctime(&now), 24);
    c_time[24] = '\0';
    sprintf(c_buf, "%s %s �פJ�¹ڹ�(%d)\n", c_time, acct.userid, (int)m1);
    f_cat(FN_BANK, c_buf);

    pmsg2("�¹ڹ��פJ����");
     
    unlink(buf);	
    close(fd);

    return 0;    
	
  }
  
  return 0;
  
}


int bank_main()
{
  char buf[2];
  int money;
  int point1;

  ACCT acct;
  if(acct_load(&acct, cuser.userid) >= 0)
    {
      money = acct.money;
      point1 = acct.point1;      
    }
  else
    {
      pmsg2("�d�L�z���b���T..."); 
      return 0;
    }

  clear();

  move(0,0);
  prints("\033[1;33;42m                            " BOARDNAME "    �Ȧ�                                   \033[m\n\n");
  move(10,0);
  prints("  �z���b����T�p�U    ��. ���u�}�I�ƴ����ڹ�\n\n");
  prints("                      ��. �ιڹ������u�}�I��\n\n");
  prints("                      ��. ���\n\n");
  prints("                      ��. �״ڵ���L�H\n");
  prints("\n");
             
  move (4,2);
  prints("�ڹ� %d ",money);
  move (6,2);
  prints("�u�}�n�� %d ",point1);
  if(!vget(b_lines,0,"�п�ܱz�n���A�ȡG [Q] ���} ",buf,2,DOECHO))
    return 0;

  if(*buf == '1')
    point1_money();
  else if(*buf == '2')
    pmsg2("���\\��|���}��");
  else if(*buf == '3')
    pmsg2("���\\��|���}��");
  else if(*buf == '4')
    TransferAccount();
  else
    pmsg2("���}�Ȧ�");    

  return 0;
}
