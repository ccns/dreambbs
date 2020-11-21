/* ----------------------------------------------------- */
/* chrono ==> file name (32-based)                       */
/* 0123456789ABCDEFGHIJKLMNOPQRSTUV                      */
/* ----------------------------------------------------- */

#include "dao.h"

const char radix32[32] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
};

void archiv32(time_t chrono,    /* 32 bits */
              char *fname        /* 7 chars */
    )
{
    char *str;

    str = fname + 7;
    *str = '\0';
    for (;;)
    {
        *(--str) = radix32[chrono % 32U];
        if (str == fname)
            return;
        chrono /= 32U;
    }
}

void archiv32m(time_t chrono,    /* 32 bits */
               char *fname        /* 7 chars */
    )
{
    char *str;

    str = fname + 8;
    *str = '\0';
    for (;;)
    {
        *(--str) = radix32[chrono % 32U];
        if (str == fname)
            return;
        chrono /= 32U;
    }
}

GCC_PURE time_t chrono32(const char *str  /* M0123456 */
    )
{
    unsigned long chrono;
    unsigned int ch;

    chrono = 0;
    while ((ch = *++str))
    {
        ch -= '0';
        if (ch >= 10)
            ch -= 'A' - '0' - 10;
        chrono = (32 * chrono) + ch;
    }
    return chrono;
}
