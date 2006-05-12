#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "sys/wcebase.h"

void
_DEFUN (__assert, (file, line, failedexpr),
	const char *file _AND
	int line _AND
	const char *failedexpr)
{
  (void)fiprintf(stderr,
	"assertion \"%s\" failed: file \"%s\", line %d\n",
	failedexpr, file, line);

  char buf[512];
  wchar_t wbuf[512];

  snprintf(buf, sizeof(buf)-1, 
	  "assertion \"%s\" failed: file \"%s\", line %d\n",
	  failedexpr, file, line);
  buf[sizeof(buf)-1] = 0;

  MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, COUNTOF(wbuf));

  MessageBoxW(NULL, wbuf, _T("Assert"), MB_OK);
  DebugBreak();
  abort();
  /* NOTREACHED */
}
