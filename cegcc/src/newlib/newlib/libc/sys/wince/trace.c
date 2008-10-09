#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <alloca.h>

#include "sys/wcetrace.h"
#include "sys/wcefile.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef CE_NOTRACE

static HANDLE __wcetracehnd = NULL;
static int __wcetrace = 0;
static int __wcetrace_debugger = 0;	/* Used to be WCE_ALL */
static int __trace_closed = 0;

void
WCETRACESET(int trace)
{
  if (!__trace_closed)
    __wcetrace = trace;
}

int
WCETRACEGET(void)
{
  return(__wcetrace);
}

void
WCETRACE_DEBUGGER_SET(int trace)
{
  __wcetrace_debugger = trace;
}

int
WCETRACE_DEBUGGER_GET(void)
{
  return(__wcetrace_debugger);
}

typedef struct 
{
  const char* str;
  int flag;
} trace_entry;

/* This define specifies how long a buffer is needed for displaying
   all the levels.  It should be the sum of all names of level
   lengths, plus the amount of level lengths (for the separation
   commas) plus 6 (currently) digits for displaying the numeric value
   of the level.  And then add a terminating 0, and a safety padding.
   The WCE_ALL is not accounted for, as it consumes all other levels,
   and never shows in the output.  */

#define MAX_LEVEL_BUF   (41 + 9 + 6 + 10)

static const trace_entry trace_entries[] = 
{
  { "all", WCE_ALL }, 

  { "io", WCE_IO },                     /* 2    = 2 */
  { "network", WCE_NETWORK },           /* 7    = 9 */
  { "signals", WCE_SIGNALS },           /* 7    = 16 */
  { "fifos", WCE_FIFOS },               /* 5    = 21 */
  { "time", WCE_TIME },                 /* 4    = 25 */
  { "synch", WCE_SYNCH },               /* 5    = 30 */
  { "malloc", WCE_MALLOC },             /* 6    = 36 */
  { "vm", WCE_VM },                     /* 2    = 38 */
  { "app", WCE_APP },                   /* 3    = 41 */

  { NULL, 0 }
};

/* Convert LEVEL into a comma separated string, and store it in TO.
   TO must be at least MAX_LEVEL_BUF long.  */

static void
get_level_name(int level, char *to)
{
  int virgin = 1;

  const trace_entry *ent = trace_entries + 1;
  for (; ent->str; ent++)
    {
      if (level & ent->flag)
	{
	  int len;

	  level &= ~ent->flag;
	  if (!virgin)
	    *(to++) = ',';
	  else
	    virgin = 0;

	  len = strlen(ent->str);
	  memcpy(to, ent->str, len);
	  to += len;
        }
    }

  if (level)
    {
      int printed;

      /* There were some unresolved flags.  */
      if (!virgin)
	*(to++) = ',';

      printed = sprintf(to, "%#04.4x", level);
      to += printed;
    }

  *to = 0;
}

void NKDbgPrintfA(const char *fmt, ...);

static void set_from_env(const char* env, int* what)
{
  char buf[512];
  const char *trace = getenv(env);
  if (!trace)
  {
    NKDbgPrintfA("\"%s\" not found in registry\n", env);
    return;
  }
  else
  {
    NKDbgPrintfA("parsing: \"%s\":\"%s\"\n", env, trace);
  }

  *what = 0;
  strcpy(buf, trace);

  const trace_entry* entry;
  const char *p;

  for (p = strtok(buf, ":"); p; p = strtok(NULL, ":")) {
    NKDbgPrintfA("option token \"%s\"\n", p);
    int neg = 0;
    if (p[0] == '-' && p[1] != '\0') {
      NKDbgPrintfW(L"neg option\n");
      p++;
      neg = 1;
    }

    NKDbgPrintfW(L"checking option\n");

    for (entry = trace_entries; entry->str; entry++) {
      if (!strcmp(p, entry->str)) {
        NKDbgPrintfW(L"valid option found.\n");
        if (neg)
          *what &= ~entry->flag;
        else
          *what |= entry->flag;
        break;
      }
    }
  }
  NKDbgPrintfW(L"no more tokens\n");
}

void
WCETRACEGETENV(void)
{
  set_from_env("WCETRACE", &__wcetrace);
  set_from_env("WCETRACE_DEBUGGER", &__wcetrace_debugger);
}

void
WCETRACECLOSE(void)
{
  if (__wcetracehnd != NULL && __wcetracehnd != INVALID_HANDLE_VALUE)
  {
    __wcetrace = WCE_IO;
    __WCETrace(WCE_IO, "Trace file is being closed");
    FlushFileBuffers(__wcetracehnd);
    XCECloseHandle(__wcetracehnd);
    __wcetracehnd = NULL;
    __trace_closed = 1;
    __wcetrace = 0;
  }
}

void
__WCETrace(int level, const char *fmt, ...)
{
  int len;
  char level_name[MAX_LEVEL_BUF];
  char buf[1024];

  va_list ap;

  if (!(__wcetrace_debugger & level) && !(__wcetrace & level))
    return;

  get_level_name(level, level_name);
  len = sprintf(buf, "%08X:%08X: [%s] ",
		GetTickCount(), GetCurrentThreadId(),
		level_name);

  va_start(ap, fmt);
  vsprintf(buf + len, fmt, ap);
  strcat(buf, "\n");

  if (__wcetrace & level) {
    if (__wcetracehnd == NULL) {
      char tracepath[256];
      wchar_t tracepathw[256];
      const char* tmppath = getenv("TMP");
      int pid = getpid();
      NKDbgPrintfW(L"pid is %d (%x)\n", pid, pid);
      if (!tmppath || strlen(tmppath) == 0)
        sprintf(tracepath, "/Temp/wcetrace%u.log", pid);
      else 
      {
        len = strlen(tmppath);
        if (tmppath[len-1] == '\\' || tmppath[len-1] == '/')
          sprintf(tracepath, "%swcetrace%u.log", tmppath, pid);
        else
          sprintf(tracepath, "%s/wcetrace%u.log", tmppath, pid);
      }

      mbstowcs(tracepathw, tracepath, strlen(tracepath) + 1);
      __wcetracehnd = XCECreateFileW(tracepathw, GENERIC_WRITE, 
        FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
    }

    if(__wcetracehnd != INVALID_HANDLE_VALUE) {
      int nwritten;
      WriteFile(__wcetracehnd, buf, strlen(buf), (DWORD *)&nwritten, NULL);
      FlushFileBuffers(__wcetracehnd);
    }
  }

  if (__wcetrace_debugger & level)
  {
    len = strlen(buf) + 1;
    wchar_t *wbuf = alloca(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP,0, buf, len, wbuf, len);
    OutputDebugStringW(wbuf);
  }
}

void
__WCETraceError(int trace, DWORD error, const char* func)
{
  wchar_t* wbuf;
  int len;
  char* buf;

  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    error,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (wchar_t*) &wbuf,
    0, NULL);

  len = wcslen(wbuf);
  buf = alloca(len + 1);
  wcstombs(buf, wbuf, len + 1);
  LocalFree(wbuf);
  WCETRACE(trace, "%s failed with error %d: %s", func, (int)error, buf); 
  printf("%s failed with error %d: %s\n", func, (int)error, buf); 
}

#endif /* CE_NOTRACE */
