#include "bbs.h"
#include <string.h>

int main(int argc, char *argv[])
{
    printf("\x1b[1;33mCopying %s to %s ....\x1b[0m\n",argv[1],argv[2]);
    f_mv(argv[1],argv[2]);
    return 0;
}
