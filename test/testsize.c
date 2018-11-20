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
    printf("sizeof(BRH): %zu\n",sizeof(BRH));
    printf("sizeof(HDR): %zu\n",sizeof(HDR));
    printf("sizeof(CLASS): %zu\n",sizeof(CLASS));
    printf("sizeof(ACCT): %zu\n",sizeof(ACCT));
    //printf("sizeof(): %zu\n",);

    return 0;
}
