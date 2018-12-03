/*-------------------------------------------------------*/
/* util/hdr-dump.c      ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : 看板標題表                                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
/* Usage  : hdr-dump .DIR                                */
/*-------------------------------------------------------*/


#include "bbs.h"
int
main(
    int argc,
    char *argv[])
{
    int inf, count;
    HDR hdr;

    if (argc < 2)
    {
        printf("Usage:\t%s .DIR\n", argv[0]);
        exit(1);
    }

    inf = open(argv[1], O_RDONLY);
    if (inf == -1)
    {
        printf("error open file\n");
        exit(1);
    }

    count = 0;
    while (read(inf, &hdr, sizeof(hdr)) == sizeof(hdr))
    {
        count++;
        printf("%04d\t%c/%s\t%s\t%s\t%s\t%s\n", count, hdr.xname[7], hdr.xname, hdr.owner, hdr.nick, hdr.date, hdr.title);
    }
    close(inf);

    exit(0);
}
