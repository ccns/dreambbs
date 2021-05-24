#include "bbs.h"
#include "test.h"

#if __STDC_VERSION__ >= 201112L  /* C11 */ || __cplusplus >= 201103L  /* C++11 */
  #define EXPECT_SIZE(Type, expect) \
    static_assert(sizeof(Type) == expect, #Type " check failed!")
#else
  #define EXPECT_SIZE(Type, expect)  assert(sizeof(Type) == expect)
#endif

#define SHOW_SIZE(Type) do { \
    printf("sizeof(" #Type "): %zu\n", sizeof(Type)); \
} while (0)

#define CHECK_SIZE(Type, expect)  do { \
    SHOW_SIZE(Type); \
    EXPECT_SIZE(Type, expect); \
} while (0)

int main(void)
{
    // print out variable size
    SHOW_SIZE(size_t);
    SHOW_SIZE(int);
    SHOW_SIZE(long);
    SHOW_SIZE(time_t);

    //print out bbs struct size
    CHECK_SIZE(BRD, 256);
    CHECK_SIZE(BRH, 12);
    CHECK_SIZE(HDR, 256);
    CHECK_SIZE(ACCT, 512);

    return 0;
}
