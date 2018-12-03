#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bbs.h"
int main(void)
{
    int fd, n;
    SCHEMA *usr;
    struct stat st;
    struct tm *tm_t;
    char userid[IDLEN+1];

    if((fd = open(BBSHOME "/.USR", O_RDONLY)) < 0)
    {
        printf("ERROR at open file");

        exit(1);
    }
    fstat(fd, &st);
    usr = malloc(st.st_size);
    read(fd, usr, st.st_size);
    close(fd);
    printf("\nst.st_size=%d\n", st.st_size);
    for(n=0;n < (st.st_size/sizeof(SCHEMA)) ; n++)
    {
        tm_t = localtime(&usr[n].uptime);
        strncpy(userid, usr[n].userid, IDLEN);
        userid[IDLEN]='\0';
        printf("uptime:%d/%d userid:%-12s\n",
            tm_t->tm_mon, tm_t->tm_mday, userid);

        if(n % 25 == 0)
            fd=getchar();
    }
}





