#include "bbs.h"
#include <string.h>

int main(int argc, char *argv[])
{
    if(argc!=3) {
        exit(1);
    }
    char *source = argv[1];
    char *destination = argv[2];
    fprintf(stderr, "\x1b[1;33mCopying %s to %s ....\x1b[0m\n", source, destination);
    f_cp(source, destination, 0600);
    return 0;
}
