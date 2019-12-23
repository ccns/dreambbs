#include "config.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *fp;
    unsigned int a;

    if (argc !=2)
        return 2;

    fp = fopen(argv[1], "r");
    if (fp)
    {
        do
        {
            fscanf(fp, "%x", &a);
            printf("%u.%u.%u.%u\n", (a&0xFF), (a&0x0000FF00)>>8, (a&0x00FF0000)>>16, (a&0xFF000000)>>24);
        } while (!feof(fp));
        fclose(fp);
    }
    return !fp;
}
