#include "dao.h"

GCC_PURE time_t chrono32(const char *str  /* M0123456 */
    )
{
    time_t chrono;
    int ch;

    chrono = 0;
    while ((ch = *++str))
    {
        ch -= '0';
        if (ch >= 10)
            ch -= 'A' - '0' - 10;
        chrono = (chrono << 5) + ch;
    }
    return chrono;
}
