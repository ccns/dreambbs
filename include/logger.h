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

#endif // LOGGER_H
