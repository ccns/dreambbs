/*-------------------------------------------------------*/
/* checkbrd.c                                            */
/*-------------------------------------------------------*/
/* author : cache.bbs@bbs.ee.ncku.edu.tw                 */
/* target : �ˬd brd �U�� .DIR                           */
/* create : 09/12/18                                     */
/* update :                                              */
/*-------------------------------------------------------*/

//�Х��ƥ��Ҧ����, ���ˬd�O�_�sDIR.o���c�O�_�s�b

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

    setgid(BBSGID);
    setuid(BBSUID);

    for (c = 'a'; c <= 'z'; c++)
    {
        char buf[64];
        GCC_UNUSED char buf2[64], buf3[64];
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

            if ((argc == 2) && str_casecmp(str, argv[1]))
                continue;

            sprintf(buf, "%s/" FN_DIR, str);
            if ((fd = open(buf, O_RDONLY)) < 0)
            {
                printf("brd/%s is missing, cp DIR.o\n", buf);
                //sprintf(buf2, "%s/.DIR.o", str);
                //sprintf(buf3, "%s/.DIR", str);
                //PROC_CMD("/bin/cp", buf2, buf3);
            }
            else
                continue;


        }
            printf("brd/%c is done.\n", c);

        closedir(dirp);
    }

    return 0;
}
