#include "config.h"

#include <stdio.h>
#include <string.h>
#include "dao.h"
#include "test.h"

int main(void)
{
    char t[]="Hello";
    str_xor(t, "he3");
    assert(!strcmp(t, " e_lo"));
    return 0;
}

