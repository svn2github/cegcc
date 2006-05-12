/*
FUNCTION
<<system>>---execute command string

INDEX
	system
INDEX
	_system_r

ANSI_SYNOPSIS
	#include <stdlib.h>
	int system(char *<[s]>);

	int _system_r(void *<[reent]>, char *<[s]>);

TRAD_SYNOPSIS
	#include <stdlib.h>
	int system(<[s]>)
	char *<[s]>;

	int _system_r(<[reent]>, <[s]>)
	char *<[reent]>;
	char *<[s]>;

DESCRIPTION

Use <<system>> to pass a command string <<*<[s]>>> to <</bin/sh>> on
your system, and wait for it to finish executing.

Use ``<<system(NULL)>>'' to test whether your system has <</bin/sh>>
available.

The alternate function <<_system_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
<<system(NULL)>> returns a non-zero value if <</bin/sh>> is available, and
<<0>> if it is not.

With a command argument, the result of <<system>> is the exit status
returned by <</bin/sh>>.

PORTABILITY
ANSI C requires <<system>>, but leaves the nature and effects of a
command processor undefined.  ANSI C does, however, specify that
<<system(NULL)>> return zero or nonzero to report on the existence of
a command processor.

POSIX.2 requires <<system>>, and requires that it invoke a <<sh>>.
Where <<sh>> is found is left unspecified.

Supporting OS subroutines required: <<_exit>>, <<_execve>>, <<_fork_r>>,
<<_wait_r>>.
*/

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <reent.h>

#include "sys/wceprocess.h"

__IMPORT int XCEExecuteProcessA(const char *commandline, BOOL bWait, LPDWORD lpdwProcId);

int _system_r (struct _reent *ptr, const char *s)
{
	if (!s)
		return XCEExecuteProcessA("/bin/sh", 1, 0);
  else
  {
    char cmdbuf[1024];
    sprintf(cmdbuf, "/bin/sh %s", s);
    return XCEExecuteProcessA(cmdbuf, 1, 0);
  }
}
