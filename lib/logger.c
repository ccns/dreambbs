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
#include <string.h>
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
 *     otherwise output the message to `stderr`.
 * Messages with its log level >= `logger->lv_skip` will be ignored.
 * An extra newline will be appended to the message for output,
 *     therefore it is not needed to include a trailing newline in `format`. */
GCC_FORMAT(3, 4) GCC_NONNULL(1, 3)
void loggerf(const Logger *logger, enum LogLevel level, const char *format, ...)
{
    if (level < 0 || level >= logger->lv_skip)
        return;

    /* Temporarily open the file if necessary */
    FILE *const file = logger->file ? logger->file : logger->path ? fopen(logger->path, "a") : stderr;

    if (!file)
        return;

    const int fd = fileno(file);

    va_list args;
    va_start(args, format);

    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", localtime(&TEMPLVAL(time_t, {time(NULL)})));

    f_exlock(fd);

    fprintf(file, "%s [%s] %d: ", buf, loglv_name[level], INT(getpid()));
    vfprintf(file, format, args);
    fputc('\n', file);
    fflush(file);

    f_unlock(fd);

    va_end(args);

    /* Close the temporarily opened file */
    if (!logger->file)
        fclose(file);
}

/* The default formatter for message logging with tag */
static void logger_tag_formatter_default(char *buf, size_t size, const char *tag, const char *msg)
{
    snprintf(buf, size, "[%s] %s", tag, msg);
}

/* Log a message with tag and custom formatter to the file specified by `tlogger`
 * Use `tlogger->logger.file` if it is not `NULL`,
 *     otherwise temporarily open and then close the file with path `tlogger->logger.path` if it is not `NULL`,
 *     otherwise output the message to `stderr`.
 * If `tlogger->formatter` is `NULL`, the default formatter is used.
 * All messages are not ignored.
 * An extra newline will be appended to the message for output,
 *     therefore it is not needed to add a trailing newline in the formatter. */
GCC_NONNULLS
void logger_tag(const TLogger *tlogger, const char *tag, const char *msg)
{
    const Logger *const logger = &tlogger->logger;
    FILE *const file = logger->file ? logger->file : logger->path ? fopen(logger->path, "a") : stderr;

    if (!file)
        return;

    void (*const formatter)(char *buf, size_t size, const char *mode, const char *msg) =
        tlogger->formatter ? tlogger->formatter : logger_tag_formatter_default;
    const int fd = fileno(file);

    char buf[512];

    formatter(buf, sizeof(buf) - 1, tag, msg); // Reserve 1 byte for the trailing newline
    strcat(buf, "\n");
    write(fd, buf, strlen(buf));

    if (!logger->file)
        fclose(file);
}
