#include "config.h"

#include <unistd.h>
#include "dao.h"

int xwrite(int fd, const char *data, int size)
{
    while (size > 0)
    {
        int cc = write(fd, data, size);
        if (cc < 0)
            return cc;
        data += cc;
        size -= cc;
    }
    return 0;
}
