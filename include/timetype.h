/*-------------------------------------------------------*/
/* timetype.h   ( NCKU CCNS WindTop-DreamBBS 3.21 )      */
/*-------------------------------------------------------*/
/* Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>    */
/* Target: Helper functions for fixed-size `time_t`      */
/* Create: 2021-02-23                                    */
/*-------------------------------------------------------*/

#ifndef TIMEDEF_H
#define TIMEDEF_H

#include <time.h>

#include "attrdef.h"
#include "cppdef.h"

/* Integral `time_t` types */

typedef int32_t time32_t;
typedef uint32_t utime32_t;
typedef int64_t time64_t;
typedef uint64_t utime64_t;

/* Helper functions for fixed-size `time_t` */

static inline time32_t time32(time32_t *tloc)
{
    if (tloc)
        return *tloc = (time32_t)(utime32_t)time(NULL);
    return (time32_t)(utime32_t)time(NULL);
}

static inline time64_t time64(time64_t *tloc)
{
    if (tloc)
        return *tloc = time(NULL);
    return time(NULL);
}

/* Helper macros for for fixed-size `time_t` */

#define ctime_any(_timep) \
    ctime(&TEMPLVAL(time_t, {*(_timep)}))

#define ctime_any_r(_timep, _buf) \
    ctime_r(&TEMPLVAL(time_t, {*(_timep)}), _buf)

#define gmtime_any(_timep) \
    gmtime(&TEMPLVAL(time_t, {*(_timep)}))

#define gmtime_any_r(_timep, _result) \
    gmtime_r(&TEMPLVAL(time_t, {*(_timep)}), _result)

#define localtime_any(_timep) \
    localtime(&TEMPLVAL(time_t, {*(_timep)}))

#define localtime_any_r(_timep, _result) \
    localtime_r(&TEMPLVAL(time_t, {*(_timep)}), _result)

#endif  // #ifndef TIMEDEF_H
