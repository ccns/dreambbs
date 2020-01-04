#include "bbs.h"
#include <assert.h>

#if __STDC_VERSION__ >= 201112L  /* C11 */ || __cplusplus >= 201103L  /* C++11 */
  #define EXPECT_SIZE(Type, expect) \
    static_assert(sizeof(Type) == expect, #Type " check failed!")
#else
  #define EXPECT_SIZE(Type, expect)  assert(sizeof(Type) == expect)
#endif

#define CHECK_SIZE(Type, expect)  do { \
    printf("sizeof(" #Type "): %zu\n", sizeof(Type)); \
    EXPECT_SIZE(Type, expect); \
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
