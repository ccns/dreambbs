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
  unsigned char *dst; /* Thor.990409: 任意長度任意binary seq, 至少要 src那麼長*/
  unsigned char *src; /* Thor.990409: 任意長度str, 不含 \0 */
                      /* Thor: 結果是將src xor到dst上, 若有0結果, 則不變 ,
			       所以dst長度必大於等於 src(以字串而言) */
            
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
