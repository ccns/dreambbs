#include "bbs.h"
#include <string.h>

int main(int argc, char *argv[])
{
    fprintf(stderr,"\x1b[1;33mCopying %s to %s ....\x1b[0m\n",argv[1],argv[2]);
    f_cp(argv[1],argv[2],0600);
    return 0;
}
