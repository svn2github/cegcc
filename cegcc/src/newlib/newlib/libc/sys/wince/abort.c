#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

_VOID
_DEFUN_VOID (abort)
{
#ifdef ABORT_MESSAGE
  write (2, "Abort called\n", sizeof ("Abort called\n")-1);
#endif
  _exit (1);
}
