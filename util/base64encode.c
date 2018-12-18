/*-------------------------------------------------------*/
/* util/base64encode.c  ( YZU WindTopBBS Ver 3.00 )      */
/*-------------------------------------------------------*/
/* target :                                              */
/* create :                                              */
/* update :                                              */
/*-------------------------------------------------------*/

#include "bbs.h"

/* ----------------------------------------------------- */
/* encode BASE64                                         */
/* ----------------------------------------------------- */


void
base64_encode(FILE *in, FILE *out)
{
    char *ascii = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char base[3], *dst;
    char dest[73];
    unsigned char c1, c2, c3;
    int n;

    dst = dest;

    while ((n=fread(base, 1, sizeof(base), in)))
    {

        c1 = base[0];
        c2 = base[1];
        c3 = base[2];
        *dst++ = ascii[c1 >> 2];
        *dst++ = ascii[(c1 & 0x3) << 4 | (c2 & 0xf0) >> 4];
        if (n==1)
        {
            *dst++ = '=';
            *dst++ = '=';
            break;
        }
        else if (n==2)
        {
            *dst++ = ascii[(c2 & 0xf) << 2 | (c3 & 0xc0) >> 6];
            *dst++ = '=';
            break;
        }
        else
        {
            *dst++ = ascii[(c2 & 0xf) << 2 | (c3 & 0xc0) >> 6];
            *dst++ = ascii[(c3 & 0x3f)];
        }
        if ((dst - dest) >= 72)
        {
            *dst = '\0';
            fprintf(out, "%s\n", dest);
            dst = dest;
        }
    }
    if ((dst - dest) > 0)
    {
        *dst = '\0';
        fprintf(out, "%s\n", dest);
    }
}


int
main(
    int argc,
    char *argv[])
{

    if (argc > 1)
    {
        fclose(stdin);
        stdin = fopen(argv[1], "r");
    }
    if (stdin)
        base64_encode(stdin, stdout);
    else
        fprintf(stdout, "error open file\n");

    return 0;
}
