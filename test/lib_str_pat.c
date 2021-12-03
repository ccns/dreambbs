#include "config.h"

#include <stdio.h>
#include "dao.h"
#include "test.h"

#define STR_PAT(x, y)  (void) (printf("<%s, %s> : %d\n", x, y, str_pat(x, y)), assert(str_pat(x, y)))

int main(void)
{
    STR_PAT("a", "a*");
    STR_PAT("abc", "a*");
    STR_PAT("abc", "a*c");
    STR_PAT("abc", "a?c");
    STR_PAT("level", "l*l");
    STR_PAT("level", "l*e*l");
    STR_PAT("lelelelel", "l*l*l*l");
    return 0;
}
