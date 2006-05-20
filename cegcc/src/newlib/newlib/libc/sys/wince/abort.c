#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "sys/wcebase.h"

_VOID
_DEFUN_VOID (abort)
{
#ifdef ABORT_MESSAGE
  write (2, "Abort called\n", sizeof ("Abort called\n")-1);
#endif

  wchar_t buf[256];
  GetModuleFileNameW(NULL, buf, sizeof(buf));
  MessageBoxW(0, buf, L"Abort!", 0);
  DebugBreak();
  TerminateProcess(0,0);
  _exit (1);

  while (1)
    {
      raise (SIGABRT);
	  DebugBreak();
      _exit (1);
    }
}
