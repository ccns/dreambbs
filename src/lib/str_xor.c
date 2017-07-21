/*-------------------------------------------------------*/ 
/* lib/str_xor.c     ( NTHU CS MapleBBS Ver 3.10 )   	 */ 
/*-------------------------------------------------------*/ 
/* author : thor.bbs@bbs.cs.nthu.edu.tw			 */
/* target : included C for str xor-ing (signed mail)	 */ 
/* create : 99/03/30                                     */ 
/* update :   /  /                                       */ 
/*-------------------------------------------------------*/ 
 
//unsigned char *
void
str_xor(dst, src)
  unsigned char *dst; /* Thor.990409: ���N���ץ��Nbinary seq, �ܤ֭n src�����*/
  unsigned char *src; /* Thor.990409: ���N����str, ���t \0 */
                      /* Thor: ���G�O�Nsrc xor��dst�W, �Y��0���G, �h���� ,
			       �ҥHdst���ץ��j�󵥩� src(�H�r��Ө�) */
            
{
  register int cc;
  for(; *src; src++, dst++)
  { 
    if ((cc = *src ^ *dst))
      *dst = cc;
  } 
} 


#if 0
#include<stdio.h>
main()
{
  char t[]="Hello";
  printf(str_xor(t,"he3"));
}
#endif
