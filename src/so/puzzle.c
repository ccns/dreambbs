/*-------------------------------------------------------*/
/* so/puzzle.c          ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* author : Ernie					 */
/* modify : visor.bbs@bbs.yzu.edu.tw			 */
/* target : ���z���L		                         */
/* create : 02/03/98                                     */
/* update : 08/06/2000                                   */
/*-------------------------------------------------------*/
/* note : ���{���ѭ�������� ghill ���� 		 */
/*-------------------------------------------------------*/

#include "bbs.h"

int pad[4][4];
char pic[16][19];
     

static int 
rule(void)
{
   char buf[3];

   clear();
   vs_bar("�Ϯ׫��L");
   move(1, 0);
   outs(
"\n\
\n\
        �w��Ӫ��Ϯ׫��L�I\n\
        �п�@�i���w���Ϯ׬D�ԧa�G\n\
\n\
        [1;33;41m �s��  �Ϯ�      ���׫���   [m\n\
        [1m  ��   �^��r��  ��         [m (���ע��A�ժ�)\n\
          ��   ĵ�    ������     [m (�`�N�p�a���IBy stanley)\n\
          ��   �d�ϻP��  ������      (�����O����I  By aquarian)\n\
          ��   Snoopy    ����������  (���VSnoopy�D�ԶܡH)\n\
          ��\n\
          ��\n\
          ��\n\
          ��\n\
          ��\n\
          ��\n\
\n\
        �p�G�A���������Ϯ�(��x�e=12x24)\n\
        �w��bNote�O��Z��I\n\
");

   while(1)
   {
      if(!vget(22, 0,"�z�n����i��[0-9]�A���}[q]?", buf, 3, LCECHO))
        return -1;
      if(*buf == 'q')
         return -1;
      else if(buf[0] >= '0' && buf[0] <= '3')   /* �}��Ϯ� */
         return (*buf - '0');
      else
      	 vmsg("��J���~  �Э��s��J");
   }
}

static void 
suck_in_pic(which_pic)
  int which_pic;
{
   int i,j,k;
   char *pos;
   char pic_data[301];

   memset(*pic,0,16*19);
/*   resolve_pad(which_pic, pic_data);*/
   
/*   resolve_film(which_pic+PAD_MOVIE,0,pic_data);*/
   i = open("etc/pad",O_RDONLY);
   read(i,pic_data,300);
   close(i);

   pos = pic_data;
   for(j=0;j<=3;j++)
   {
     for(i = 0; i <= 2; i++)              /* �N�Ϥ����Φ�16�p�� */
     {
       for(k=0;k<=3;k++)
       {
         strncat(pic[j*4+k], pos, 6);
         pos += 6;
       }
       pos++;
     }
   }

}

static void 
init_pad()
{
   int i, j, k, flag;
   
   flag = 0;

   for(i = 0; i <= 3; i++)
      for(j = 0; j <= 3; j++)
      {
         do
         {
            k = rand() % 16;
            if(flag == 0x0000ffff)
               return;
         }while(flag & (1 << k));

         pad[i][j] = k;
         flag |= (1 << k);
      }
}

static void 
draw_part(a,b,tmp)
  int a,b;
  char *tmp;
{
   int i;
   char buf[7];

   memset(buf,0,7);
   for(i = 0; i <= 2; i++)
   {
      strncpy(buf,(tmp+i*6),6);
      move(i + 4 + 3*a, 10 + 6*b);
      prints("%6s",buf);
   }
}

static void 
draw_pad(draw_complete)
  int draw_complete;
{
   int i, j;

   for(i = 0; i <= 3; i++)
     for(j = 0; j <= 3; j++)
     {
       if(pad[i][j] || draw_complete)
       {
         move(4 + 3*i, 10 + 6*j);
         if(pad[i][j])
           draw_part(i, j, pic[pad[i][j]-1]);
         else
           draw_part(i, j, pic[15]);   
       }
       else
       {
         move(4 + 3*i, 10 + 6*j); outs("      ");
         move(5 + 3*i, 10 + 6*j); outs("      ");
         move(6 + 3*i, 10 + 6*j); outs("      ");
       }
     }
}

int 
NumPad()
{
   int count = 0;
   int i = 0, j = 0, tmp, a, b = 0, which_pic;
   char buf[5];
   time_t start;
   
   utmp_mode(M_XMODE);
   for(i = 0; i <= 3; i++)
      for(j = 0; j <= 3; j++)
      {
         if(b > 15)
            b = -1;
         pad[i][j]= ++b;
      }
      
   do
   {
      which_pic = rule();
      if(which_pic == -1)
         return 0;
      else
      {
         suck_in_pic(which_pic);
         clear();
         move(3, 0);
         outs("        [1;33m�z�w�w�w�w�w�w�w�w�w�w�w�w�{[m");
         draw_pad(1);
         move(16, 0);
         outs("        [1;33m�|�w�w�w�w[37;45m ������ [0;1;33m�w�w�w�w�}[m");
         vget(18, 0, "�ݲM�������Ϫ��ˤl�S�H[y/N]", buf, 3, LCECHO);
      }
   }while(*buf != 'y');

   vs_head("�Ϯ׫��L",str_site);
   srand(time(NULL));
   init_pad();

   for(a = 0; a <= 3; a++)              /* look for the 0 */
   {
      for(b = 0; b <= 3; b++)
      {
         if(pad[a][b] == 0)
         {
            i = a;
            j = b;
            break;
         }
      }
   }

   move(3, 0);
   outs("        [1;33m�z�w�w�w[37;44m������������[0;1;33m�w�w�w�{[m");
   draw_pad(0);
   move(16, 0);
   outs("        [1;33m�|�w�w�w�w[37;44m[q] ����[0;1;33m�w�w�w�w�}[m");

   start = time(NULL);

   while(1)
   {
      move(b_lines - 5, 0);
      prints("              ���ʨB�ơG%4d �B\n", count);
      prints("              ��O�ɶ��G%4d ��\n", time(NULL) - start);
      if(pad[0][0] == 1 && pad[0][1] == 2 && pad[0][2] == 3 && pad[0][3] == 4 &&
         pad[1][0] == 5 && pad[1][1] == 6 && pad[1][2] == 7 && pad[1][3] == 8 &&
         pad[2][0] == 9 && pad[2][1] == 10 && pad[2][2] == 11 && pad[2][3] == 12 &&
         pad[3][0] == 13 && pad[3][1] == 14 && pad[3][2] == 15 && pad[3][3] == 0)
      {
         vmsg("�z�㧹���F�I�n�F�`�I�I");
         return 0;
      }

      switch(vkey())
      {
         case KEY_DOWN:
            if(i - 1 >= 0)
            {
               tmp = pad[i][j];
               pad[i][j] = pad[i - 1][j];
               pad[i - 1][j] = tmp;
               draw_pad(0);
               i--;
               count++;
            }
            break;

         case KEY_UP:
            if(i + 1 <= 3)
            {
               tmp = pad[i][j];
               pad[i][j] = pad[i + 1][j];
               pad[i + 1][j] = tmp;
               draw_pad(0);
               i++;
               count++;
            }
            break;

         case KEY_RIGHT:
            if(j - 1 >= 0)
            {
               tmp = pad[i][j];
               pad[i][j] = pad[i][j - 1];
               pad[i][j - 1] = tmp;
               draw_pad(0);
               j--;
               count++;
            }
            break;

         case KEY_LEFT:
            if(j + 1 <= 3)
            {
               tmp = pad[i][j];
               pad[i][j] = pad[i][j + 1];
               pad[i][j + 1] = tmp;
               draw_pad(0);
               j++;
               count++;
            }
            break;

         case 'q':
         case 'Q':
            vget(b_lines - 2, 0, "�u���n���}�H[y/N]", buf, 3, LCECHO);
            if(*buf == 'y')
            {
               char gameover[100];
               sprintf(gameover,"�w�� %s �A�Ӫ��ڡI", cuser.userid);
               vmsg(gameover);
               return 0;
            }
      }
   }
   return 0;
}
