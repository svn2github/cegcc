#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <errno.h>

#include "sys/wcetypes.h"
#include "sys/wceerror.h"
#include "sys/wcetrace.h"

#if 0

// moved to startup.c
// calling _initexit is
// crashing crashing on longjmp
// replaced _initexit call with setjmp directly and the crash goes away...
// leave these here to remember latter that there is work 
// to be done in this area.

static jmp_buf exitjmp;
static int exitcode;

int
_initexit()
{
  return setjmp(exitjmp);
}

int
_getexitcode()
{
  return(exitcode);
}

void
_setexitcode(int code)
{
  exitcode = code;
}

void
_exit(int code)
{
//  WCETRACE(WCE_IO, "_exit: %d", code);
  exitcode = code;
  longjmp(exitjmp, 1);
}

#endif
