#include "bbs.h"
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc!=2) {
        exit(1);
    }
    char *source = argv[1];
    fprintf(stderr, "\x1b[1;33mRemoving %s ....\x1b[0m\n", source);
    f_rm(source);
    return 0;
}
