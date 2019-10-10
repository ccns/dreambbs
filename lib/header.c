#include "dao.h"
#include "modes.h"
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

void hdr_fpath(char *fpath, const char *folder, const HDR * hdr)
{
    char *str = NULL;
    int cc, chrono;

    while ((cc = *folder++))
    {
        *fpath++ = cc;
        if (cc == '/')
            str = fpath;
    }

    chrono = hdr->chrono;
    folder = hdr->xname;
    cc = *folder;
    if (cc != '@')
        cc = radix32[chrono & 31];

    if (*str == '.')
    {
        *str++ = cc;
        *str++ = '/';
    }
    else
    {
        str[-2] = cc;
    }

    if (hdr->xmode & GEM_EXTEND)
    {
        *str++ = 'X';
        archiv32(chrono, str);
    }
    else
        strcpy(str, hdr->xname);
}

/* ----------------------------------------------------- */
/* hdr_stamp - create unique HDR based on timestamp      */
/* ----------------------------------------------------- */
/* fpath - directory                                     */
/* token - A / F / 0                                     */
/* ----------------------------------------------------- */
/* return : open() fd (not close yet) or link() result   */
/* ----------------------------------------------------- */

int hdr_stamp(const char *folder, int token, HDR * hdr, char *fpath)
{
    char *fname, *family = NULL;
    int rc, chrono;
    char buf[128];
    const char *flink;

    flink = NULL;
    if (token & (HDR_LINK | HDR_COPY))
    {
        flink = fpath;
        fpath = buf;
    }

    fname = fpath;
    while ((rc = *folder++))
    {
        *fname++ = rc;
        if (rc == '/')
            family = fname;
    }
    if (*family != '.')
    {
        fname = family;
        family -= 2;
    }
    else
    {
        fname = family + 1;
        *fname++ = '/';
    }

    if ((rc = token & 0xdf))    /* ÅÜ¤j¼g */
    {
        *fname++ = rc;
    }
    else
    {
        *fname = *family = '@';
        family = ++fname;
    }

    chrono = time(0);

    for (;;)
    {
        *family = radix32[chrono & 31];
        archiv32(chrono, fname);

        if (flink)
        {
            if (token & HDR_LINK)
                rc = f_ln(flink, fpath);
            else
                rc = f_cp(flink, fpath, O_EXCL);
        }
        else
        {
            rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);
        }

        if (rc >= 0)
        {
            memset(hdr, 0, sizeof(HDR));
            hdr->chrono = chrono;
            str_stamp(hdr->date, &hdr->chrono);
            strcpy(hdr->xname, --fname);
            break;
        }

        if (errno != EEXIST)
            break;

        chrono++;
    }

    return rc;
}
