/*-------------------------------------------------------*/
/* util/topuser.c         (MapleBBS Ver 3.00 )           */
/*-------------------------------------------------------*/
/* author : gslin@abpe.org                               */
/*          mat.bbs@fall.twbbs.org                       */
/* target : �W�����ơB������ơB�����ɼƱƦ�]           */
/* create : 99/03/29                                     */
/* update : 99/03/29                                     */
/*-------------------------------------------------------*/
/* syntax : topuser                                      */
/*-------------------------------------------------------*/

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bbs.h"

DATA toplogins[TOPNUM], topposts[TOPNUM], topstay[TOPNUM];

int
sort_compare(
    const void *p1,
    const void *p2)
{
    const DATA *a1, *a2;

    a1 = (const DATA *) p1;
    a2 = (const DATA *) p2;

    return (a2->num-a1->num);
}

DATA *
findmin(
    DATA *src)
{
    int i;
    DATA *p;

    p = src;
    for (i = 0; i < TOPNUM; i++)
        if (src[i].num < p->num)
            p = src+i;
    return (p);
}

void
merge_id_nick(
    char *dst,
    char *userid,
    char *nick)
{
    if (*userid)
    {
        sprintf(dst, "%s (%s)", userid, nick);

        if (strlen(dst) > 25)
            dst[25] = 0;
    }
    else
        dst[0] = 0;
}

void
write_data(
    const char *title,
    DATA *data,
    int mode)
{
    char buf[256];
    int i;
    const char *color;

    sprintf(buf, "\x1b[1;32m%%%ds\x1b[m\n\n", 80);
    printf(buf, "%s", title);
    if (mode == 0)
    {
        puts("\x1b[1;31m�W��\x1b[m  \x1b[1;33mID\x1b[m (\x1b[1;34mNickname\x1b[m)  "
"           \x1b[1;36m����\x1b[m    \x1b[1;31m�W��\x1b[m  \x1b[1;33mID\x1b[m (\x1b[1;34m"
"Nickname\x1b[m)             \x1b[1;36m����\x1b[m\n-----------------------------"
"-------    ------------------------------------");
    }
    else
    {
        puts("\x1b[1;31m�W��\x1b[m  \x1b[1;33mID\x1b[m (\x1b[1;34mNickname\x1b[m)  "
"           \x1b[1;36m�ɼ�\x1b[m    \x1b[1;31m�W��\x1b[m  \x1b[1;33mID\x1b[m (\x1b[1;34m"
"Nickname\x1b[m)             \x1b[1;36m�ɼ�\x1b[m\n-----------------------------"
"-------    ------------------------------------");
    }

    for (i = 0; i < TOPNUM_HALF; i++)
    {
        char buf1[80], buf2[80];

        merge_id_nick(buf1, data[i].userid, data[i].username);
        merge_id_nick(buf2, data[i+TOPNUM_HALF].userid,
                      data[i+TOPNUM_HALF].username);
        switch(i)
        {
            case 0:
                color = "\x1b[1;31m";
                break;
            case 1:
                color = "\x1b[1;32m";
                break;
            case 2:
                color = "\x1b[1;33m";
                break;
            default:
                color = "\x1b[m";
        }

        printf("%s[%2d]  %-25s%5d\x1b[m    [%2d]  %-25s%5d\n", color, i+1, buf1, data[i].num,
               i+TOPNUM_HALF+1, buf2, data[i+TOPNUM_HALF].num);
    }

    printf("\n");
}

int
main(void)
{
    char c;

    memset(toplogins, 0, sizeof(toplogins));
    memset(topposts, 0, sizeof(topposts));
    memset(topstay, 0, sizeof(topstay));

    for (c = 'a'; c <= 'z'; c++)
    {
        char buf[64];
        struct dirent *de;
        DIR *dirp;

        sprintf(buf, BBSHOME "/usr/%c", c);
        chdir(buf);

        if (!(dirp = opendir(".")))
            continue;

        while ((de = readdir(dirp)))
        {
            ACCT cuser;
            DATA *p;
            int fd;

            if (de->d_name[0] <= ' ' || de->d_name[0] == '.')
                continue;

            sprintf(buf, "%s/.ACCT", de->d_name);
            if ((fd = open(buf, O_RDONLY)) < 0)
                continue;

            read(fd, &cuser, sizeof(cuser));
            close(fd);

            if ((p = findmin(toplogins))->num < cuser.numlogins)
            {
                strncpy(p->userid, cuser.userid, IDLEN);
                strncpy(p->username, cuser.username, 24);
                p->num = cuser.numlogins;
            }

            if ((p = findmin(topposts))->num < cuser.numposts)
            {
                strncpy(p->userid, cuser.userid, IDLEN);
                strncpy(p->username, cuser.username, 24);
                p->num = cuser.numposts;
            }

            if ((p = findmin(topstay))->num < (cuser.staytime/3600))
            {
                strncpy(p->userid, cuser.userid, IDLEN);
                strncpy(p->username, cuser.username, 24);
                p->num = cuser.staytime/3600;
            }
        }
    }

    qsort(toplogins, TOPNUM, sizeof(DATA), sort_compare);
    qsort(topposts, TOPNUM, sizeof(DATA), sort_compare);
    qsort(topstay, TOPNUM, sizeof(DATA), sort_compare);

    chdir(BBSHOME);

    write_data("\x1b[33;1m===========\x1b[44m    �W�����ƱƦ�]    \x1b[40m============\x1b[m", toplogins, 0);
    write_data("\x1b[33;1m===========\x1b[44m    ������ƱƦ�]    \x1b[40m============\x1b[m", topposts, 0);
    write_data("\x1b[33;1m===========\x1b[44m    �����ɼƱƦ�]    \x1b[40m============\x1b[m", topstay, 1);
    return 0;
}
