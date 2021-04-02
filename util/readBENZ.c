#include "bbs.h"

int main(void)
{
    char c;

    setgid(BBSGID);
    setuid(BBSUID);

    for (c = 'a'; c <= 'z'; c++)
    {
        char buf[64]/*, rmbuf[64]*/;
        struct dirent *de;
        DIR *dirp;

        sprintf(buf, BBSHOME "/usr/%c", c);
        chdir(buf);

        if (!(dirp = opendir(".")))
            continue;

        while ((de = readdir(dirp)))
        {
            int fd;
            BMW bmw;
            if (de->d_name[0] <= ' ' || de->d_name[0] == '.')
                continue;
            sprintf(buf, "%.*s/%s", IDLEN, de->d_name, FN_BENZ);
            if ((fd = open(buf, O_RDWR)) < 0)
                continue;

            while (read(fd, &bmw, sizeof(bmw)))
            {
                printf("ID:%-*s sender:%-4d recver:%-4d msg:%s\n",
                       IDLEN, bmw.userid, bmw.sender, bmw.recver, bmw.msg);
            }
        }
    }
}
