#include "bbs.h"
#include "test.h"

int main(int argc, char *argv[])
{
    if (argc!=2) {
        exit(2);
    }
    char *source = argv[1];
    fprintf(stderr, "\x1b[1;33mRemoving %s ....\x1b[0m\n", source);
    assert(!f_rm(source));
    return 0;
}
