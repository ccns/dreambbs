/*-------------------------------------------------------*/
/* lib/passwd.c ( NCKU CCNS WindTop-DreamBBS 3.20 )      */
/*-------------------------------------------------------*/
/* Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>    */
/* Target: Password hashing library for DreamBBS         */
/* Create: 2020/11/21                                    */
/*-------------------------------------------------------*/

#define __STDC_WANT_LIB_EXT1__  1
#define _GNU_SOURCE  /* For `crypy()` & `explicit_bzero()` */
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <crypt.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dao.h"
#include "modes.h"
#include "struct.h"

#if defined __GLIBC_PREREQ && !defined __UCLIBC__
#define GLIBC_PREREQ(M, m) (__GLIBC_PREREQ(M, m))
#else
#define GLIBC_PREREQ(M, m) 0
#endif

#ifndef LIB_STRING_LINUX_HAVE_GETRANDOM
#define LIB_STRING_LINUX_HAVE_GETRANDOM (GLIBC_PREREQ(2, 25) && __linux__)
#endif

#if LIB_STRING_LINUX_HAVE_GETRANDOM
#include <sys/random.h>
#endif

#ifndef SHA256_SALT
#define SHA256_SALT "$5$"
#endif

/* ----------------------------------------------------- */
/* password encryption                                   */
/* ----------------------------------------------------- */

#ifndef LIB_STRING_HAVE_ARC4RANDOM_CHACHA20
#define LIB_STRING_HAVE_ARC4RANDOM_CHACHA20  \
    (OpenBSD >= 201405 /* 5.5 */ || __FreeBSD_version >= 1200000 /* 12.0 */)
#endif
#ifndef LIB_STRING_HAVE_EXPLICIT_BZERO
#define LIB_STRING_HAVE_EXPLICIT_BZERO \
    (GLIBC_PREREQ(2, 25) || OpenBSD >= 201405 /* 5.5 */ || __FreeBSD_version >= 1100000 /* 11.0 */)
#endif

/* IID.20190524: Get bytes from the system PRNG device. */
char *getrandom_bytes(char *buf, size_t buflen)
{
#if LIB_STRING_LINUX_HAVE_GETRANDOM
    if (getrandom(buf, buflen, GRND_NONBLOCK) == -1)
        return NULL;
#elif BSD_HAVE_ARC4RANDOM_CHACHA20
    arc4random_buf(buf, buflen);
#else
    int fd;
    if ((fd = open("/dev/urandom", O_RDONLY)) < 0)
        return NULL;
    read(fd, buf, buflen);
    close(fd);
#endif

    return buf;
}

/* IID.20190826: Write zeros to a buffer, not optimized away if possible. */
void explicit_zero_bytes(char *buf, size_t buflen)
{
#ifdef __STDC_LIB_EXT1__
    memset_s(buf, buflen, 0, buflen);
#elif LIB_STRING_HAVE_EXPLICIT_BZERO
    explicit_bzero(buf, buflen);
#else
    /* Cannot do anything better */
    memset(buf, 0, buflen);
#endif
}

static char pwbuf[PASSLEN + PASSHASHLEN];

/* `mode`: Encryption method
    `GENPASSWD_SHA256` (5): SHA-256
    `GENPASSWD_DES` (0) / otherwise: DES
*/
/* NOTE: The string pointed by `pw` will be wiped out. */
char *genpasswd(char *pw, int mode)
{
    char saltc[PASSLEN], *hash;
    int i, c;

    if (!*pw)
        return pw;

    c = 0;
    if (mode == GENPASSWD_SHA256)
    {
        memcpy(saltc, SHA256_SALT, 3);  /* IID.20190522: SHA-256. */
        c += 3;
    }

    /* IID.20190524: Get salt from the system PRNG device. */
    if (!getrandom_bytes(saltc + c, PASSLEN-1 - c))
    {
        explicit_zero_bytes(pw, strlen(pw));
        return NULL;
    }

    saltc[PASSLEN-1] = '\0';

    for (i = c; i < PASSLEN-1; i++)
    {
        c = (saltc[i] & 0x3f) + '.';
        if (c > '9')
            c += 7;
        if (c > 'Z')
            c += 6;
        saltc[i] = c;
    }
    strcpy(pwbuf, pw);
    explicit_zero_bytes(pw, strlen(pw));

    hash = crypt(pwbuf, saltc);
    explicit_zero_bytes(pwbuf, PLAINPASSLEN);
    if (!hash)
    {
        if (mode)
            return genpasswd(pw, GENPASSWD_DES);  /* Fall back to DES encryption */
        return NULL;
    }
    str_scpy(pwbuf, hash, PASSLEN + PASSHASHLEN);

    if (mode == GENPASSWD_SHA256)
        memmove(pwbuf + PASSLEN, pwbuf + PASSLEN-1, PASSHASHLEN);  /* Prefix `passhash` with `$` */
    else
        pwbuf[PASSLEN] = '\0';  /* Make `passhash` an empty string */
    pwbuf[PASSLEN-1] = '\0';
    return pwbuf;
}

/* `genpasswd` for site signature */
/* NOTE: The string pointed by `pw` will be wiped out. */
char *gensignature(char *pw)
{
    char *hash;

    hash = genpasswd(pw, GENPASSWD_SHA256);
    if (!hash)
        return NULL;

    if (hash[PASSLEN])  /* `genpasswd()` is not falled back */
    {
        memmove(hash, hash + 3, PASSLEN-1-3);  /* Remove `SHA256_SALT` prefix */
        memmove(hash + PASSLEN-1-3, hash + PASSLEN + 1, PASSHASHLEN-1-1);  /* Remove `$` prefix for `passhash` */
        hash[PASSLEN-1-3 + PASSHASHLEN-1-1] = '\0';
    }
    return hash;
}


/* Thor.990214: 註解: 合密碼時, 傳回0 */
/* NOTE: The string pointed by `test` will be wiped out. */
int chkpasswd(const char *passwd, const char *passhash, char *test)
{
    char *pw;

    /* if (!*passwd) return -1 *//* Thor.990416: 怕有時passwd是空的 */

    str_scpy(pwbuf, test, PLAINPASSLEN);
    explicit_zero_bytes(test, strlen(test));

    if (*passwd == '$')   /* IID.20190522: `passhash` is the encrypted password. */
    {
        pw = crypt(pwbuf, passwd) + PASSLEN;
        explicit_zero_bytes(pwbuf, PLAINPASSLEN);
        return (strncmp(pw, passhash+1, PASSHASHLEN));  /* `passhash` is prefixed with `$` */
    }
    else  /* IID.20190522: `passwd` is the encrypted password; legacy/falled-back `passwd`. */
    {
        pw = crypt(pwbuf, passwd);
        explicit_zero_bytes(pwbuf, PLAINPASSLEN);
        return (strncmp(pw, passwd, PASSLEN));
    }
}

/* `chkpasswd` for site signature */
/* NOTE: The string pointed by `test` will be wiped out. */
int chksignature(const char *passwd, char *test)
{
    char saltc[PASSLEN], hashc[PASSHASHLEN];

    if (strlen(passwd) <= PASSLEN-1)  /* Legacy/falled-back `passwd` */
        return chkpasswd(passwd, NULL, test);

    memcpy(saltc, SHA256_SALT, 3);  /* Restore `SHA256_SALT` prefix */
    str_scpy(saltc+3, passwd, PASSLEN-3);

    hashc[0] = '$';   /* Restore `$` prefix for `passhash` */
    str_scpy(hashc+1, passwd + PASSLEN-1-3, PASSHASHLEN-1);

    return chkpasswd(saltc, hashc, test);
}
