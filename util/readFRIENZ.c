#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bbs.h"

int main(void)
{
#if 0
    char c;

    for (c = 'a'; c <= 'z'; c++)
    {
#endif
        char buf[64]/*, rmbuf[64]*/;
        struct dirent *de;
#if 0
        DIR *dirp;

        sprintf(buf, BBSHOME "/usr/%c", c);
        chdir(buf);

        if (!(dirp = opendir(".")))
            continue;

        while (de = readdir(dirp))
        {
#endif
            int fd, max;
            BMW bmw;
#if 0
            if (de->d_name[0] <= ' ' || de->d_name[0] == '.')
                continue;
            sprintf(buf, "%s/friend", de->d_name);
#endif
            strcpy(buf, "/home/bbs/usr/p/pcbug/frienz");
            if ((fd = open(buf, O_RDWR)) < 0)
/*              continue; */exit(1);

            while (read(fd, &bmw, sizeof(bmw)))
            {
                printf("ID:%-13s sender:%-4d recver:%-4d msg:%s\n"
                      , bmw.userid, bmw.sender, bmw.recver, bmw.msg);
            }



#if 0
        }
    }
#endif
}
