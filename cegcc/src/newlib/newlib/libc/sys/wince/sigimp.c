#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "sys/wcetrace.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if 0
SIGHANDLER _sighandlers[NSIG];

SIGHANDLER
xcesignal(int sig, SIGHANDLER new)
{
  SIGHANDLER old;

  XCETRACE2(XCE_TRACE_SIGNALS, ("signal(%d, %x)", sig, new));

  if(sig < 0 || sig >= NSIG)
    return SIG_ERR;

  old = _sighandlers[sig];
  _sighandlers[sig] = new;

  return old;
}

int
xceraise(int sig)
{
  XCETrace("raise(%d)", sig);

  if(_sighandlers[sig] == SIG_IGN)
    {
      XCETrace("signal ignored");
    }
  else if(_sighandlers[sig] == SIG_DFL)
    {
      XCETrace("calling default handler");

      if(sig == SIGINT)
	exit(-1);
    }
  else
    {
      XCETrace("calling signal handler");
      (_sighandlers[sig])(sig);
      XCETrace("calling signal handler ok");
    }

  return 0;
}
#endif

int 
sigaction(int sig, struct sigaction *act,
	  struct sigaction *oldact)
{
  return 0;
}

int 
sigprocmask(int how, const sigset_t *set,
	    sigset_t *oldset)
{
  return 0;
}

#if 0
int 
sigaddset(sigset_t *mask, int sig)
{
  *mask |= (1 << sig);

  return 0;
}

int 
sigemptyset(sigset_t *mask)
{
  *mask = 0;

  return 0;
}
#endif

int 
sigsuspend(const sigset_t *sigmask)
{
  return 0;
}

int
_kill_r(struct _reent *reent, pid_t pid, int sig)
{
  HANDLE hnd;
  int    retval, winerr;

  if (sig == SIGKILL) {
    hnd = OpenProcess(0, FALSE, (unsigned int)pid);
    if (hnd == NULL) {
      errno = _winerr2errno(GetLastError());
      return(-1);
    }

    TerminateProcess(hnd, 0);
    return(0);
  }

  errno = ENOTSUP;
  return(-1);
}  
  
#if 0
int 
xcekill(pid_t pid, int sig)
{
  if(pid == xcegetpid())
    {
      xceraise(sig);
    }

  errno = EINVAL;
  return -1;
}
#endif
