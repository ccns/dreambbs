#include "bbs.h"
#include <assert.h>

#define CHECK_SIZE(Type, expect)  do { \
    printf("sizeof(" #Type "): %zu\n", sizeof(Type)); \
    static_assert(sizeof(Type) == expect, #Type " check failed!"); \
} while (0)

int main(void)
{
    // print out variable size
    CHECK_SIZE(size_t, 4);
    CHECK_SIZE(int, 4);
    CHECK_SIZE(long, 4);
    CHECK_SIZE(time_t, 4);

    //print out bbs struct size
    CHECK_SIZE(BRD, 256);
    CHECK_SIZE(BRH, 12);
    CHECK_SIZE(HDR, 256);
    CHECK_SIZE(CLASS, 36);
    CHECK_SIZE(ACCT, 512);

    return 0;
}
