/*-------------------------------------------------------*/
/* checkbrd.c                                            */
/*-------------------------------------------------------*/
/* author : cache.bbs@bbs.ee.ncku.edu.tw                 */
/* target : 檢查 brd 下的 .DIR                           */
/* create : 09/12/18                                     */
/* update :                                              */
/*-------------------------------------------------------*/

//請先備份所有資料, 並檢查是否新DIR.o結構是否存在

#include "bbs.h"

int
main(
    int argc,
    char *argv[])
{
    char c;

    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [brdname]\n", argv[0]);
        return 2;
    }

    for (c = 'a'; c <= 'z'; c++)
    {
        char buf[64];
        GCC_UNUSED char buf2[64];
        struct dirent *de;
        DIR *dirp;

        sprintf(buf, BBSHOME "/brd/%c", c);
        chdir(buf);

        if (!(dirp = opendir(".")))
            continue;

        while ((de = readdir(dirp)))
        {
            int fd;
            char *str;

            str = de->d_name;
            if (*str <= ' ' || *str == '.')
                continue;

            if ((argc == 2) && str_cmp(str, argv[1]))
                continue;

            sprintf(buf, "%s/" FN_DIR, str);
            if ((fd = open(buf, O_RDONLY)) < 0)
            {
                printf("brd/%s is missing, cp DIR.o\n", buf);
                //sprintf(buf2, "cp %s/.DIR.o %s/.DIR", str, str);
                //system(buf2);
            }
            else
                continue;


        }
            printf("brd/%c is done.\n", c);

        closedir(dirp);
    }

    return 0;
}
