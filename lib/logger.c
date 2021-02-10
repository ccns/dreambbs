/*-------------------------------------------------------*/
/* lib/logger.c ( NCKU CCNS WindTop-DreamBBS 3.0 )       */
/*-------------------------------------------------------*/
/* Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>    */
/* Target: Message logging library for DreamBBS          */
/* Create: 2021/02/06                                    */
/*-------------------------------------------------------*/

#include "config.h"

#include "logger.h"

#include <stdarg.h>
#include <time.h>

#include <sys/types.h>
#include <unistd.h>

#include "dao.h"

static const char *const loglv_name[LOGLV_COUNT] = {
    "emerg",
    "alert",
    "crit",
    "err",
    "warn",
    "notice",
    "info",
    "debug",
};

/* Log a formatted message with log level `level` using the setting of `logger`
 * Use `logger->file` if it is not `NULL`,
 *     otherwise temporarily open and then close the file with path `logger->path` if it is not `NULL`,
 *     otherwise do nothing.
 * Messages with its log level >= `logger->lv_skip` will be ignored.
 * An extra newline will be appended to the message for output,
 *     therefore it is not needed to include a trailing newline in `format`. */
GCC_FORMAT(3, 4) GCC_NONNULL(1, 3)
void loggerf(const Logger *logger, enum LogLevel level, const char *format, ...)
{
    if (level < 0 || level >= logger->lv_skip)
        return;

    /* Temporarily open the file if necessary */
    FILE *const file = logger->file ? logger->file : logger->path ? fopen(logger->path, "a") : NULL;

    if (!file)
        return;

    const int fd = fileno(file);

    va_list args;
    va_start(args, format);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", localtime(&TEMPLVAL(time_t, {time(NULL)})));

    f_exlock(fd);

    fprintf(file, "%s [%s] %d: ", buf, loglv_name[level], (int)getpid());
    vfprintf(file, format, args);
    fputc('\n', file);
    fflush(file);

    f_unlock(fd);

    va_end(args);

    /* Close the temporarily opened file */
    if (!logger->file)
        fclose(file);
}
