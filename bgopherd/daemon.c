/********************************************************************
 * MODULE: daemon.c
 * Routines to detach from and run as a daemon.
 ********************************************************************/

#ifdef unix
#  include <sys/param.h>
#endif

#include <stdio.h>
#include <errno.h>
extern int errno;

#include <signal.h>

#include "gopherd.h"

#ifndef NOFILE
#  define NOFILE (100)
#endif

/* A BSD style SIGCLD signal handler that can be used by a server that's not
   interested in its child's exit status, but needs to wait for them, to
   avoid clogging up the system with zombies.

Beware that the calling process may get an interrupted system call when we
   return, so they had better handle that.

(From Stevens pg 82) */

#include "Wait.h"

#ifdef hpux
#define TIOCNOTTY       _IO('t', 113)	/* void tty association */
#endif

void
sig_child()
{
  /* Use the waitpid() system call with the WNOHANG option */

  int pid;
  Portawait status;

  /** waitpid only once.. **/

  if ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    ActiveSessions--;
}


#ifdef SIGCHLD
static RETSIGTYPE
sig_sigchld(int signo)
{
  pid_t pid;
  Portawait status;

  if (signal(SIGCHLD, sig_sigchld) == SIG_ERR)
    err_sys("signal error");

  if ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    ActiveSessions--;

}
#endif				/* SIGCHLD */

#ifdef SIGCLD
static RETSIGTYPE
sig_sigcld(int signo)
{
  pid_t pid;
  Portawait status;

  /* pid = wait(&status); */


  if ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    ActiveSessions--;

  signal(SIGCLD, sig_sigcld);
  return;
}
#endif				/* SIGCLD */


#if defined(SIGTSTP) && !defined(M_XENIX) && !defined(USG)	/* True on a BSD system */
#include <sys/file.h>
#include <sys/ioctl.h>
#endif

/* Detach a daemon process from login session context Nonzero ignsigcld->
   nuke zombie children */

void
daemon_start(int ignsigcld)
{
  register int childpid, fd;

  /* If we were started by init (process 1) from the /etc/inittab file
     there's no need to detach. */

  if (getppid() != 1)
  {



#ifdef SIGTTOU
    signal(SIGTTOU, SIG_IGN);
#endif

#ifdef SIGTTIN
    signal(SIGTTIN, SIG_IGN);
#endif

#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
#endif

    /* If we were not started in the background, fork and let the parent
       exit.  This also guarantees the first child is not a process group
       leader. */

    if ((childpid = fork()) < 0)
      err_sys("can't fork first child");
    else if (childpid > 0)
      exit(0);			/* parent goes bye-bye */

    /* First Child process
    
    Disassociate from controlling terminal and process group. Ensure the
       process can't reacquire a new controlling terminal */

    /* Need to add defined(TIOCNOTTY) because SCO2.3.2 has SIGTSTP defined
       but is NOT BSD and doesnt have TIOCNOTTY */

#if defined(SIGTSTP) && !defined(USG) && defined(TIOCNOTTY)	/* BSD */


#if defined(HAVE_SETSID)
    setsid();
#else
    if (setpgrp(0, getpid()) == -1)
      err_sys("can't change process group");

    if ((fd = uopen("/dev/tty", O_RDWR, 000)) >= 0)
    {
      ioctl(fd, TIOCNOTTY, (char *) NULL);	/* lose controlling TTY */
      close(fd);
    }
#endif				/* hpux  et al. */

#else				/* System V */

    if (setpgrp() == -1)
      err_sys("Can't change process group");

    signal(SIGHUP, SIG_IGN);	/* immune from pgrp leader death */

    if ((childpid = fork()) < 0)
      err_sys("Can't fork second child");
    else if (childpid > 0)
      exit(0);			/* First child */

    /* second child */
#endif

  }				/* End of if test for ppid == 1 */

  /** Close any file descriptors **/

  for (fd = 0; fd < NOFILE; fd++)
    close(fd);

  errno = 0;			/* probably set to EBADF from a close */


  /** Change the current directory to / **/

  /* No need for this, since we're already where we want to be */
  /* rchdir("/"); */

  /** Clear inherited file mode creation mask. **/

  umask(0);

  /* See if the caller isn't ineterested in the exit status of its children,
     and doesn't want to have them become zombies */

  if (ignsigcld)
  {

#if defined(SIGCLD)		/* && !defined(_AIX) */
    if (signal(SIGCLD, sig_sigcld))
      perror("Signal Error..");	/* System V */
#endif

    /**  Lots of brain dead systems equate SIGCLD with SIGCHLD

         This is wrong since the semantics are different between the two

         But to insure no zombies do SIGCLD first, then SIGCHLD... **/

#if defined(SIGCHLD) && !defined(_AUX_SOURCE) && !defined(__svr4__) &&           !defined(USG) && !defined(SYSV) && !defined(hpux) && !defined(sgi) && !defined(_AIX)
    if (signal(SIGCHLD, sig_sigchld))
      perror("signal error");
#endif

  }
}
