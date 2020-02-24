/*-------------------------------------------------------*/
/* lib/proc.c   ( NCKU CCNS WindTop-DreamBBS 3.0 )       */
/*-------------------------------------------------------*/
/* Author: 37586669+IepIweidieng@users.noreply.github.com*/
/* Target: Process manipulation library for DreamBBS     */
/* Create: 2020/02/24                                    */
/*-------------------------------------------------------*/

#include "config.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include "dao.h"

/* `proc_run*()`: Run executable `path` in a child process */

int proc_runv(const char *path, const char *argv[])
{
    const pid_t pid = fork();

    switch (pid)
    {
    case 0:  // Children
        execv(path, (char **)argv);
        exit(127);  // `execv()` failed

    case -1:  // Error
        return -1;

    default:
        waitpid(pid, &SINKVAL(int), 0);
        return 0;
    }
}

/* Variadic version for convenience */
GCC_CHECK_SENTINEL(0)
int proc_runl(const char *path, const char *arg0, ...)
{
    /* Collect args first */

    int argc = 0;
    int argv_len = 8;
    const char **argv = (const char **)malloc(sizeof(const char *) * argv_len);
    const char *ptr;
    int ret;

    argv[argc++] = arg0;

    if (arg0)
    {
        va_list args;
        va_start(args, arg0);
        while ((ptr = va_arg(args, const char *)))
        {
            argv[argc++] = ptr;
            if (argc >= argv_len)
            {
                const char **argv_new = (const char **)realloc(argv, sizeof(const char *) * (argv_len *= 2));
                if (!argv_new)
                    return -1;  // `realloc()` failed
                argv = argv_new;
            }
        }
        va_end(args);

        argv[argc++] = NULL;
    }

    /* And then invoke it */

    ret = proc_runv(path, argv);
    free(argv);
    return ret;
}
