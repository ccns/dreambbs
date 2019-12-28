#include "config.h"

#include <stdio.h>
#include <assert.h>
#include "dao.h"

int main(void)
{
    char t[]="Hello";
    str_xor(t, "he3");
    assert(!str_cmp(t, " e_lo"));
    return 0;
}

