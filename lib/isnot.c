#include "config.h"

#include <string.h>
#include "dao.h"

GCC_CONSTEXPR bool is_alnum(int ch)
{
    return ((ch >= '0' && ch <= '9') ||
            (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}

GCC_CONSTEXPR bool is_alpha(int ch)
{
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}

GCC_PURE bool is_fname(const char *str)
{
    int ch;

    ch = *str;
    if (ch == '/')
        return false;

    do
    {
        if (!is_alnum(ch) && !strchr("-._/+@", ch))
            return false;
    } while ((ch = *++str));
    return true;
}

/* ----------------------------------------------------- */
/* transform to real path & security check               */
/* ----------------------------------------------------- */

int is_fpath(char *path)
{
    int level;
    char *source, *target;

    level = 0;
    source = target = path;


    for (;;)
    {
        int ch = *source;

        if (ch == '/')
        {
            int next;

            next = source[1];

            if (next == '/')
            {
                return 0;        /* [//] */
            }
            else if (next == '.')
            {
                next = source[2];

                if (next == '/')
                    return 0;    /* [/./] */

                if (next == '.' && source[3] == '/')
                {
                    /* -------------------------- */
                    /* abc/xyz/../def ==> abc/def */
                    /* -------------------------- */

                    for (;;)
                    {
                        if (target <= path)
                            return 0;

                        target--;
                        if (*target == '/')
                            break;
                    }

                    source += 3;
                    continue;
                }
            }

            level++;
        }

        *target = ch;

        if (ch == 0)
            return level;

        target++;
        source++;
    }
}

#define STRICT_FQDN_EMAIL

GCC_PURE int not_addr(const char *addr)
{
    int ch, mode;

    mode = -1;

    while ((ch = *addr))
    {
        if (ch == '@')
        {
            if (++mode)
                break;
        }

#ifdef  STRICT_FQDN_EMAIL
        else if ((ch != '.') && (ch != '-') && (ch != '_') && !is_alnum(ch))
#else
        else if (!is_alnum(ch) && !strchr(".-_[]%!:", ch))
#endif

            return 1;

        addr++;
    }

    return mode;
}
