/*-------------------------------------------------------*/
/* include/logger.h ( NCKU CCNS WindTop-DreamBBS 3.0 )   */
/*-------------------------------------------------------*/
/* Author: Wei-Cheng Yeh (IID) <iid@ccns.ncku.edu.tw>    */
/* Target: Header file for message logging               */
/* Create: 2021/02/06                                    */
/*-------------------------------------------------------*/

#ifndef LOGGER_H
#define LOGGER_H

#include "config.h"

#include <stdio.h>

enum LogLevel {
    LOGLV_EMERG,
    LOGLV_ALERT,
    LOGLV_CRIT,
    LOGLV_ERR,
    LOGLV_WARN,
    LOGLV_NOTICE,
    LOGLV_INFO,
    LOGLV_DEBUG,

    LOGLV_COUNT,

    /* Aliases for `Logger::lv_skip` */
    LOGLV_SKIP_ALL = LOGLV_EMERG,
    LOGLV_SKIP_NONE = LOGLV_COUNT,
};

typedef struct {
    FILE *file;
    const char *path; // Path to the log file
    enum LogLevel lv_skip; // The minimum log level to ignore
} Logger;

/* Tag Logger: A logger with tag and custom formatter */
typedef struct {
    Logger logger;
    void (*formatter)(char *buf, size_t size, const char *tag, const char *msg);
} TLogger;

#define TLOGGER_DEFAULT_LVSKIP(_lv_skip) \
    LISTLIT(TLogger){ \
        .logger = { \
            .file = NULL, \
            .path = NULL, /* Use the default log file (`stderr`) */ \
            .lv_skip = (_lv_skip), \
        }, \
        .formatter = NULL, /* Use the default formatter */ \
    }

#define TLOGGER_DEFAULT TLOGGER_DEFAULT_LVSKIP(LOGLV_WARN)

#endif // LOGGER_H
