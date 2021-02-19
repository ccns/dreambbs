#include "bbs.h"

int main(void)
{
    int fd, n;
    SCHEMA *usr;
    struct stat st;
    char userid[IDLEN+1];

    setgid(BBSGID);
    setuid(BBSUID);

    if ((fd = open(BBSHOME "/.USR", O_RDONLY)) < 0)
    {
        printf("ERROR at open file\n");

        exit(1);
    }
    fstat(fd, &st);
    usr = (SCHEMA *) malloc(st.st_size);
    read(fd, usr, st.st_size);
    close(fd);
    printf("st.st_size=%lld\n", (long long)st.st_size);
    for (n=0; n < (st.st_size/sizeof(SCHEMA)); n++)
    {
        str_scpy(userid, usr[n].userid, sizeof(userid));
        printf("uptime:%s userid:%-12s\n",
            Btime(&usr[n].uptime), userid);
    }
    free(usr);
}





