#include "bbs.h"
#include "test.h"

int main(int argc, char *argv[])
{
    if (argc!=3) {
        exit(2);
    }
    char *source = argv[1];
    char *destination = argv[2];
    fprintf(stderr, "\x1b[1;33mMoving %s to %s ....\x1b[0m\n", source, destination);
    assert(!f_mv(source, destination));
    return 0;
}
