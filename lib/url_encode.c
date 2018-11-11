/*-------------------------------------------------------*/ 
/* lib/url_encode.c     ( NTHU CS MapleBBS Ver 3.10 )    */ 
/*-------------------------------------------------------*/ 
/* author : thor.bbs@bbs.cs.nthu.edu.tw			 */
/* target : included C for URL encoding  		 */ 
/* create : 99/03/30                                     */ 
/* update :   /  /                                       */ 
/*-------------------------------------------------------*/ 
 
#include "dao.h"

void
url_encode(
  unsigned char *dst, /* Thor.990331: 要 src的三倍空間 */
  unsigned char *src
)
{
  for(; *src; src++)
  { 
    if(*src == ' ')
      *dst++ = '+';
    else if(*src == '/' || *src == '.' || *src == '&' || *src == '?' || *src == '=' )
      *dst++ = *src;
    else if(is_alnum(*src)) 
      *dst++ = *src; 
    else 
    { 
      register int cc = *src;
      *dst++ = '%';
      *dst++ = radix32[cc >> 4];
      *dst++ = radix32[cc & 0xf];
    } 
  } 
  *dst = '\0'; 
} 

