#include <stdio.h>
#include <sys/types.h>
#include "bbs.h"

int main(void)
{
    // print out variable size
    printf("sizeof(size_t): %zu\n",sizeof(size_t));
    printf("sizeof(int): %zu\n",sizeof(int));
    printf("sizeof(long): %zu\n",sizeof(long));
    printf("sizeof(time_t): %zu %s\n",sizeof(time_t),sizeof(time_t) == 4 ? "" : "\x1b[1;31mtime_t check failed!\x1b[0m");
    if(sizeof(time_t) != 4) return 1;

    //print out bbs struct size
    printf("sizeof(BRD): %zu\n",sizeof(BRD));
    if(sizeof(BRD) != 256) return 1;
    printf("sizeof(BRH): %zu\n",sizeof(BRH));
    if(sizeof(BRH) != 12) return 1;
    printf("sizeof(HDR): %zu\n",sizeof(HDR));
    if(sizeof(HDR) != 256) return 1;
    printf("sizeof(CLASS): %zu\n",sizeof(CLASS));
    if(sizeof(CLASS) != 36) return 1;
    printf("sizeof(ACCT): %zu\n",sizeof(ACCT));
    if(sizeof(ACCT) != 512) return 1;
    //printf("sizeof(): %zu\n",);

    return 0;
}
